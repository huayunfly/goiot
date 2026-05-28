const https = require('https');
const zlib = require('zlib');
const assert = require('assert');
const redis = require('ioredis');
const fetch = require('node-fetch');
const { group } = require('console');
const { exit } = require('process');

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

class SetDataR {
    constructor(service_name, operation, token, group_name, data_list)
    {
        this.name = service_name;
        this.operation = operation;
        this.token = token;
        this.group_name = group_name;
        this.list = data_list;
    }

    toJSON()
    {
        return {name: this.name,
            operation: this.operation,
            token: this.token,
            data: {group_name: this.group_name, list: this.list}
        };
    }
}

class GetDataP {
    constructor(service_name, operation, token, group_name, id_list, time_range, properties, batch_size = 128, batch_num = 1)
    {
        this.name = service_name;
        this.operation = 'GetDataP';
        this.token = token;
        this.group_name = group_name;
        this.id_list = id_list;
        this.time_range = time_range;
        this.properties = properties;
        this.batch_size = batch_size;
        this.batch_num = batch_num;
    }

    toJSON()
    {
        return {name: this.name,
            operation: this.operation,
            token: this.token,
            data: {group_name: this.group_name, id_list: this.id_list, time_range: this.time_range, 
                properties: this.properties, batch_size: this.batch_size, batch_num: this.batch_num }
        };
    }
}

class LoginRequest {
    constructor(service_name, operation, username, password) {
        this.name = service_name;
        this.operation = operation;
        this.username = username;
        this.password = password;
    }

    toJSON() {
        return {
            name: this.name,
            operation: this.operation,
            condition: {
                username: this.username,
                password: this.password
            }
        }
    }
}

class TouchRequest {
    constructor(service_name, operation, token) {
        this.name = service_name;
        this.operation = operation;
        this.token = token;
    }

    toJSON() {
        return {
            name: this.name,
            operation: this.operation,
            condition: { token: this.token }
        }
    }
}

// Calls the tenant service's login(), returns the token.
async function login(username, password) {
    const login_req = new LoginRequest('tenant', 'login', username, password);
    const data = JSON.stringify(login_req);
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'User-Agent': `nodejs/${process.version}`,
            'Content-Encoding': 'gzip',
            'Accept': 'application/json'
        },
        body: data
    }
    try {
        const login_body = await fetch(service_collection['tenant'], options);
        const payload = await login_body.json();
        if (payload.statusCode == '200') {

            return payload.result.token;
        }
        else {
            return null;
        }
    }
    catch (err) {
        return null;
    }
}

class RedisConnector extends DBConnector {
    constructor(connection_path, data_config, webapi) {
        super(connection_path);
        this.data_config_ = data_config;
        this.keep_running_ = true;
        this.model_ = [];
        this.model_write_ = [];
        this.webapi_ = webapi;
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
        this.db_.on('connect', () => console.log('Redis client connected to server.'));

        this.db_.on('ready', () => {
            console.log('Redis server is ready.');
            setInterval(this.refresh_data, 5000, this);
            setInterval(this.poll_data, 1000, this);
            setInterval(this.check_connection, 10000, this);
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
        const group_name = this.data_config_.name;
        for (const driver of this.data_config_.drivers) 
        {
            const driver_id = driver.id;
            const grp_driver_id = [group_name, driver_id].join('.'); 
            const id_array = [];
            const id_array_write = [];
            for (const node of driver.nodes) {
                const address = node.address;
                for (const data of node.data) {
                    if (data.register.toUpperCase().includes('R')) {
                        id_array.push([driver_id, address, data.id].join('.'));
                    }
                    else if (data.register.toUpperCase().includes('W')) {
                        id_array_write.push([driver_id, address, data.id].join('.'));
                    }
                }
            }
            this.model_.push({ grp_name: group_name, driver_id: driver_id, id_array: id_array });
            this.model_write_.push({ grp_name: group_name, driver_id: driver_id, id_array: id_array_write });
        }
    }

    // Callback function with obj as the parameter.
    async refresh_data(obj) {
        for (const table of obj.model_) {
            // Pipeline is a kind of Multi without transaction.
            const id_list = [];
            const pipeline = obj.db_.pipeline();
            for (const data_id of table.id_array) {
                id_list.push(data_id);               
                pipeline.hmget('refresh:' + data_id, 'value', 'result', 'time');
            }
            const reply = await pipeline.exec((err, replies) => {
                if (err) {
                    console.err('Referesh data from redis failed.');
                    return;
                }
            });
            const data_list = [];
            let id_no = 0;
            const t_last = Date.now() / 1000.0 - 15.0;
            for (const item of reply)
            {
                let t_data = Number(item[1][2]);
                if (!Number.isNaN(t_data) && t_data > t_last) {
                    data_list.push({
                        "id": id_list[id_no], "value": item[1][0],
                        "result": item[1][1], "time": t_data
                    });
                }
                id_no++;
            }

            const data_list1 = [{ "id": "mfcpfc.1.pv", "value": "11.0", "result": "0", "time": 1677154221.821000 },
            { "id": "mfcpfc.2.pv", "value": "12.1", "result": "0", "time": 1677154221.839000 }];
            if (data_list.length > 0 && obj.webapi_.token) {
                const body = new SetDataR('service_name', 'SetDataR', obj.webapi_.token, table.grp_name, data_list);
                obj.webapi_.post(body);
            }
        }
    }

    // Callback function for polling data from databus api.
    async poll_data() {
        for (const table of this.model_write_) {
            const id_list = [];
            for (const data_id of table.id_array) {
                id_list.push(data_id);
            }
            if (id_list.length > 0 && this.webapi_.token) {
                let batch_count = Math.ceil(id_list.length / 128);
                for (let batch_num = 0; batch_num < batch_count; batch_num++) {
                    // One batch = 128 items
                    const id_list_batch = id_list.slice(batch_num * 128, (batch_num + 1) * 128);
                    const time_last = Date.now() / 1000.0
                    const time_range = [time_last - 15.0, time_last];
                    const properties = ['value', 'result', 'time'];
                    // Call databus api to get data, using a constant batch size of 128 to control the request size.
                    const body = new GetDataP('service_name', 'GetDataP', this.webapi_.token, table.grp_name, id_list_batch, time_range, properties, 128, 1);
                    const response = await this.webapi_.post(body);
                    if (response?.statusCode == '200' && response.result?.data?.table?.id?.length > 0) {
                        if (response.result.data.table.id.length != response.result.data.table.value.length ||
                            response.result.data.table.id.length != response.result.data.table.result.length ||
                            response.result.data.table.id.length != response.result.data.table.time.length
                        ) {
                            console.error('Data size mismatch.');
                            continue;
                        }
                        const pipeline = this.db_.pipeline();
                        for (let i = 0; i < response.result.data.table.id.length; i++) {
                            const data_id = response.result.data.table.id[i];
                            const value = response.result.data.table.value[i];
                            const result = response.result.data.table.result[i];
                            const time = response.result.data.table.time[i];
                            if (!data_id || !value || !result || Number.isNaN(Number(time))) {
                                console.error('Invalid data found.');
                                continue;
                            }
                            pipeline.hmset('poll:' + data_id, 'value', value, 'result', result, 'time', time);
                        }
                        const reply = await pipeline.exec((err, replies) => {
                            if (err) {
                                console.err('Write data to redis failed.');
                                return;
                            }
                        });
                    }
                }
            }
        }
    }

    async check_connection(obj)
    {
        const is_ok = await obj.webapi_.touch();
        if (!is_ok)
        {
            obj.webapi_.login();
        }
    }
}

class WebServiceConnector {
    constructor(api_path, username, password) {
        if (!api_path) {
            throw 'null api_path';
        }
        // check api_path
        this.api_path_ = api_path;
        this.username_ = username;
        this.password_ = password;
        this.token = null;
    }

    async login() {
        const login_request = new LoginRequest('tenant', 'login', this.username_, this.password_);
        const response = await this.post(login_request);
        if (response?.statusCode == '200') 
        {
            this.token = response.result.token;
        }
        else 
        {
            this.token = null;
        }
    }

    async touch() {
        const touch_request = new TouchRequest('tenant', 'touch', this.token);
        const response = await this.post(touch_request);
        if (response?.statusCode == '200') 
        {
            return true;
        }
        else 
        {
            return false;
        }
    }

    async post(data_obj) {
        const data_str = JSON.stringify(data_obj);
        const options = {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'User-Agent': `nodejs/${process.version}`,
                'Content-Encoding': 'gzip',
                'Accept': 'application/json'
            },
            body: data_str
        };

        try {
            const response = await fetch(this.api_path_, options);
            const body = await response.json();
            console.log("Fetch statusCode %s", body.statusCode);
            return body;
        } catch (err) {
            console.log(`Fetch error. ${err}`);
            return null;
        }
    }
}

module.exports.RedisConnector = RedisConnector;
module.exports.WebServiceConnector = WebServiceConnector;