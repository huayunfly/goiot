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

class RedisConnector extends DBConnector {
    constructor(connection_path, data_config, webapi) {
        super(connection_path);
        this.data_config_ = data_config;
        this.keep_running_ = true;
        this.model_ = [];
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
            for (const node of driver.nodes) {
                const address = node.address;
                for (const data of node.data) {
                    if (!data.register.toUpperCase().startsWith('W')) {
                        id_array.push([driver_id, address, data.id].join('.'));
                    }
                }
            }
            this.model_.push({ grp_name: group_name, driver_id: driver_id,  id_array: id_array });
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
            if (data_list1.length > 0) {
                const body = new SetDataR('service_name', 'SetDataR',
                    '0bff36ada4d1456c965ba12288b1f97d', table.grp_name, data_list);
                obj.webapi_.post('', body);
            }
        }
    }
}

class WebServiceConnector {
    constructor(api_path) {
        if (!api_path)
        {
            throw 'null api_path';
        }
        // check api_path
        this.api_path_ = api_path;
    }

    async post(service_path, data_obj) {
        const data = JSON.stringify(data_obj);
        const options = {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'User-Agent': `nodejs/${process.version}`,
                'Content-Encoding': 'gzip',
                'Accept': 'application/json'
            },
            body: data
        };

        // zlib.gzip(data, (err, buffer) => {
        //     const req = https.request(options, res => {
        //         console.log(`状态码: ${res.statusCode}`);
        //         res.setEncoding('utf8'); // Note: not requesting or handling compressed response 
        //         res.on('data', d => {
        //             // ... do stuff with returned data
        //         })
        //     });

        //     req.on('error', err => {
        //         console.error('problem with request: ' + err.message);
        //     });

        //     req.write(buffer);
        //     req.end();
        // });

        // fetch with "compress:true" by default
        (async () => {
            try {
                const result = await fetch(this.api_path_ + service_path, options);
                const body = await result.json();
                console.log('Fetch statusCode %s', body.statusCode);
            } catch (e) {
                console.log('Fetch error.');
            }
        })();
    }
}

module.exports.RedisConnector = RedisConnector;
module.exports.WebServiceConnector = WebServiceConnector;