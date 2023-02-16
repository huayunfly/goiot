const https = require('https');
const zlib = require('zlib');
const assert = require('assert');
const redis = require('ioredis');


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

class RedisConnector extends DBConnector {
    constructor(connection_path, data_config) {
        super(connection_path);
        this.data_config_ = data_config;
        this.keep_running_ = true;
        this.model_ = new Map();
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
        const name = this.data_config_.name;
        const timestamp = new Date().getTime() / 1000.0;
        for (const driver of this.data_config_.drivers) {
            const driver_id = driver.id;
            for (const node of driver.nodes) {
                const address = node.address;
                for (const data of node.data) {
                    const id = [driver_id, address, data.id].join('.');
                    let ratio = 1.0;
                    let fvalue = parseFloat(data.ratio);
                    if (!isNaN(fvalue)) {
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
                    this.model_.set(id, new DataInfo(id, data.name, 1/*dtype*/, read_or_write, ratio, 0, -1, timestamp));           
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
            const data_info = this.model_.get(id);
            if (!is_poll || (is_poll && data_info.privilege != D_PRIVILEGE.READ_ONLY))
            {
                const ns_id = key_namespace + id;
                let reply = await this.db_.hmset(ns_id, 'id', data_info.id,
                    'name', data_info.name, 'dtype', data_info.dtype, 'privilege', data_info.privilege,
                    'value', data_info.value, 'result', data_info.result, 'timestamp', data_info.timestamp);
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