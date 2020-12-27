const https = require('https');
const zlib = require('zlib');
const assert = require('assert');
const redis = require('redis');

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
        this.db_ = redis.createClient(port, host);
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
        const name = this.data_config_.name;
        for (const driver of this.data_config_.drivers) {
            const driver_id = driver.id;
            const data_array = [];
            for (const node of driver.nodes) {
                const address = node.address;
                for (const data of node.data) {
                    if (!data.register.toUpperCase().startsWith('W')) {
                        data_array.push([driver_id, address, data.id].join('.'));
                    }
                }
            }
            this.model_.push({ driver_id: driver_id, data_array: data_array });
        }

    }

    refresh_data(obj) {
        for (const table of obj.model_) {
            const table_name = table.driver_id;
            // Batch is a kind of Multi without transaction.
            const batch = obj.db_.batch();
            for (const data of table.data_array) {
                batch.hmget('refresh:' + data, 'value', 'result');
            }
            batch.exec((err, replies) => {
                if (err) {
                    console.err('Referesh data from redis failed.');
                    return;
                }
                // Value is valid if Result is '0'. 
                //obj.sqldb_.insert(table_name, table.data_array, replies.map(n => Number(n[1]) == 0 ? n[0] : 'NULL'));
                this.webapi_.post('/goiot/data', {token: null, name: 'SetData', data: []});
            });
        }
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