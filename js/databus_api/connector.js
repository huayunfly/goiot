// @date 2023.03.03
const https = require('https');
const zlib = require('zlib');
const assert = require('assert');
const redis = require('ioredis');
const { time } = require('console');
const fetch = require('node-fetch');


const D_PRIVILEGE =
{
    'READ_ONLY': 0,
    'WRITE_ONLY': 1,
    'READ_WRITE': 2,
}

const D_NAMESPACE =
{
    'NS_REFRESH': 'refresh:',
    'NS_POLL': 'poll:',
    'NS_REFRESH_TIME': 'time_r:',
    'NS_POLL_TIME': 'time_p:',
}

class DBConnector {
    constructor(connection_path) {
        this.connection_path_ = connection_path;
        this.db_ = null;
    }

    open() {

    }

    close() {

    }
}

class DataModel {
    constructor() {
        this.model_ = new Map();
    }

    // Adds a new element with a specified key and value to the Map. 
    // If an element with the same key already exists, the element will be updated.
    add(key, value) {
        this.model_.set(key, value);
    }

    // Get value by key or return undefined.
    query(key) {
        return this.model_.get(key)
    }

    // Return boolean.
    has(key) {
        return this.model_.has(key);
    }

    // Return keys iterator.
    keys() {
        return this.model_.keys();
    }
}

class RedisConnector extends DBConnector {
    constructor(connection_path, data_config) {
        super(connection_path);
        this.data_config_ = data_config;
        this.keep_running_ = true;
        this.model_ = new DataModel();
        this.open();
    }

    open() {
        // Parse arguments.
        const vars = this.connection_path_.split(':');
        assert.strictEqual(2, vars.length);
        const host = vars[0];
        const port = Number(vars[1]);
        // Create model
        this.create_model();
        // Create redis client.
        this.db_ = new redis(port, host);
        // If the connection fails and an error handler is supplied, 
        // the Redis client will attempt to retry the connection.
        this.db_.on('connect', () => {
            console.log('Redis client connected to server.');
            this.add_redis_set(D_NAMESPACE.NS_REFRESH_TIME, D_NAMESPACE.NS_REFRESH, false);
            this.add_redis_set(D_NAMESPACE.NS_POLL_TIME, D_NAMESPACE.NS_POLL, true)
        });

        this.db_.on('ready', () => {
            console.log('Redis server is ready.');
        });
        // If an error event is fired and no error handler is attached, 
        // the application process will throw the error and crash.
        this.db_.on('error', err => console.error('Redis error', err));
    }

    close() {
        console.log('redis_connector close()');
    }

    create_model() {
        if (!this.data_config_) {
            throw 'Undefined data configuration.';
        }
        const name = this.data_config_.name??'dummy';
        const t_now = Date.now() / 1000.0;
        for (const driver of this.data_config_.drivers) {
            const driver_id = driver.id;
            for (const node of driver.nodes) {
                const address = node.address;
                for (const data of node.data) {
                    const id = [name, driver_id, address, data.id].join('.');
                    let ratio = 1.0;
                    let fvalue = Number(data.ratio);
                    if (!Number.isNaN(fvalue)) {
                        ratio = fvalue;
                    }
                    let read_or_write = D_PRIVILEGE.READ_ONLY;
                    if (data.register?.startsWith('RW'))
                    {
                        read_or_write = D_PRIVILEGE.READ_WRITE;
                    }
                    else if (data.register?.startsWith('W'))
                    {
                        read_or_write = D_PRIVILEGE.WRITE_ONLY;
                    }
                    this.model_.add(id, new DataInfo(id, data.name, 8/*dtype*/, read_or_write, ratio, 0, -1, t_now));           
                }
            }
        }
    }

    async add_redis_set(time_namespace, key_namespace, is_poll)
    {
        if (!time_namespace || !key_namespace)
        {
            throw new Error('Parameter is empty.');
        }
        const existed_keys = await this.db_.zrange(time_namespace, 0, -1); // withscores
        const data_info_ids = new Set(this.model_.keys());
        for (let key of existed_keys)
        {
            key = key.substring(key_namespace.length);
            if (data_info_ids.has(key))
            {
                data_info_ids.delete(key);
            }
        }
        let add_num = 0;
        for (let id of data_info_ids)
        {
            const data_info = this.model_.query(id);
            if (!is_poll || (is_poll && data_info.privilege != D_PRIVILEGE.READ_ONLY))
            {
                const ns_id = key_namespace + id;
                let reply = await this.db_.hmset(ns_id, 'id', data_info.id,
                    'name', data_info.name, 'type', data_info.type, 'rw', data_info.privilege,
                    'value', data_info.value, 'result', data_info.result, 'time', data_info.timestamp);
                this.check_redis_reply(id, 'add_redis_set', reply);
                reply = await this.db_.zadd(time_namespace, data_info.timestamp, ns_id);
                this.check_redis_reply(id, 'zadd_redis_set', reply);
                add_num++;
            }
        }
        const zone = is_poll ? 'poll' : 'refresh';
        if (add_num > 0)
        {
            console.log(`Adding ${add_num} ${zone} set to redis finished`);
        }
    }

    check_redis_reply(id, op_name, reply)
    {
        if (reply != 'OK' && reply != 1)
        {
            console.log(`Redis reply error: [${id}] [${op_name}] ${reply}`);
        }
    }

    // Return the in-memory data model.
    data_model ()
    {
        return this.model_;
    }

    // @param <namespace> set data namespace indicating manipulation, poll or refresh.
    // @param <data_list> data list.
    // Return updated data number.
    async update_data(namespace, data_list) 
    {
        if (!data_list?.length)
        {
            return 0;
        }

        // namespace
        let key_namespace, time_namespace;
        if (namespace == D_NAMESPACE.NS_REFRESH) 
        {
            key_namespace = D_NAMESPACE.NS_REFRESH;
            time_namespace = D_NAMESPACE.NS_REFRESH_TIME;
        }
        else if (namespace == D_NAMESPACE.NS_POLL) 
        {
            key_namespace = D_NAMESPACE.NS_POLL;
            time_namespace = D_NAMESPACE.NS_POLL_TIME;
        }
        else 
        {
            throw 'Unsupported namespace.';
        }

        const batch_size = 64;
        let ok_num = 0;
        for (let i = 0; i < data_list.length;) 
        {
            let start = i;
            let end = start + batch_size;
            if (end > data_list.length)
            {
                end = start + data_list.length % batch_size;
            }
            i = end;
            const selected = data_list.slice(start, end);
            const transaction = this.db_.multi();
            for (const data of selected) 
            {
                let ns_id = key_namespace + data.id;
                transaction.hmset(
                    ns_id, 'value', data.value, 'result', data.result, 'time', data.time);
                transaction.zadd(time_namespace, data.time, ns_id);
            }
            // Each response follows the format `[err, result]`.
            const reply = await transaction.exec();
            for (const item of reply) 
            {
                if (item[1] == 'OK') {
                    ok_num++;
                }
            }
        }
        return ok_num;
    }

    // Get data from redis.
    // @param <namespace> get data namespace indicating manipulation, poll or refresh.
    // @param <group_name> group name like goiot.
    // @param <id_list> data ID with 'group_name.' prefix. Empty array covers all data.
    // @param <time_range> start and stop time array in second. [0, -1] covers all time.
    // @return A data list whose ID removed <group_name>.
    async get_data(namespace, group_name, id_list, time_range, props, batch_num, batch_size)
    {
        const data_list = [];
        // Initialize time_range [now - 5s, now]
        let t_end = Date.now() / 1000;
        let t_start = t_end - 5.0;
        if (time_range?.length && time_range.length >= 2)
        {
            t_end = parseFloat(time_range[1]);
            t_start = parseFloat(time_range[0]);
        }
        // Initialize properties
        if (!props?.length)
        {
            props = ['value', 'result', 'time'];
        }
        // namespace
        let key_namespace, time_namespace;
        if (namespace == D_NAMESPACE.NS_REFRESH)
        {
            key_namespace = D_NAMESPACE.NS_REFRESH;
            time_namespace = D_NAMESPACE.NS_REFRESH_TIME;
        }
        else if (namespace == D_NAMESPACE.NS_POLL)
        {
            key_namespace = D_NAMESPACE.NS_POLL;
            time_namespace = D_NAMESPACE.NS_POLL_TIME;
        }
        else
        {
            throw 'Unsupported namespace.';
        }
        // ZRANGEBYSCORE 
        const id_by_time = await this.db_.zrangebyscore(time_namespace, t_start, t_end);
        const match_ids = [];
        // ID matching
        if (!id_list?.length)
        {
            match_ids.push(...id_by_time);
        }
        else
        {
            const set_by_time = new Set(id_by_time);
            id_list.forEach(x => {
                const new_id = key_namespace + x;
                if (set_by_time.has(new_id))
                {
                    match_ids.push(new_id);
                }
            });
        }
        // Batch read with default value.
        if (batch_size <= 0)
        {
            batch_size = 128;
        }
        // Batch number with default value 1 (start).
        if (batch_num <= 0)
        {
            batch_num = 1;
        }
        const DOMAIN_LEN = key_namespace.length + group_name.length + 1/* dot */;
        for (let i = (batch_num - 1) * batch_size; i < match_ids.length;) 
        {
            let start = i;
            let end = start + batch_size;
            if (end > match_ids.length)
            {
                end = start + match_ids.length % batch_size;
            }
            i = end;
            const selected = match_ids.slice(start, end);
            const transaction = this.db_.multi();
            for (const ns_id of selected) 
            {
                transaction.hmget(ns_id, props);
            }
            const reply = await transaction.exec();
            for (let i = 0; i < reply.length; i++)
            {
                const new_data = {};
                new_data['id'] = selected[i].slice(DOMAIN_LEN);
                for (let j = 0; j < props.length; j++)
                {
                    if (props[j] == 'time')
                    {
                        new_data[props[j]] = parseFloat(reply[i][1][j]);
                    }
                    else
                    {
                        new_data[props[j]] = reply[i][1][j];
                    }
                }
                data_list.push(new_data);
            }
            break; // One batch read finished.
        }
        return {'group_name': group_name, 'list': data_list, 'total': match_ids.length};    
    }
}

class DataInfo {
    constructor(id, name, dtype, privilege, ratio, value, result, timestamp) {
        this.id = id;
        this.name = name;
        this.type = dtype;
        this.privilege = privilege;
        this.ratio = ratio;
        this.value = value;
        this.result = result;
        this.timestamp = timestamp;
    }
}

class WebServiceConnector {
    constructor(hostname, port, path) {
        this.hostname_ = hostname;
        this.port_ = port;
    }

    post(service_path, data_obj) {
        const data = JSON.stringify(data_obj);
        const options = {
            hostname: this.hostname_,
            port: this.port_,
            path: service_path,
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Content-Encoding': 'gzip',
            }
        };

        zlib.gzip(data, (err, buffer) => {
            const req = https.request(options, res => {
                console.log(`状态码: ${res.statusCode}`);
                res.setEncoding('utf8'); // Note: not requesting or handling compressed response
                res.on('data', d => {
                    // ... do stuff with returned data
                })
            });

            req.on('error', err => {
                console.error('problem with request: ' + err.message);
            });

            req.write(buffer);
            req.end();
        });
    }
}

module.exports.RedisConnector = RedisConnector;
module.exports.WebServiceConnector = WebServiceConnector;
module.exports.D_NAMESPACE = D_NAMESPACE;
module.exports.D_PRIVILEGE = D_PRIVILEGE;