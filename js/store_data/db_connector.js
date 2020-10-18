// https://github.com/NodeRedis/node-redis

const redis = require('redis');
const pg = require('pg');
const assert = require('assert');
const { SSL_OP_EPHEMERAL_RSA } = require('constants');
const { runInThisContext } = require('vm');

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
        this.model_ = [];
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

        this.db_.on('ready', () => { console.log('Redis server is ready.'), this.refresh_data() });
        //  If an error event is fired and no error handler is attached, 
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
            this.model_.push({driver_id: driver_id, data_array: data_array });
        }
    }

    refresh_data() {
        for (const table of this.model_) {
            const driver_id = table.driver_id;
            // Batch is a Multi without transaction.
            const batch = this.db_.batch();
            for (const data of table.data_array) {
                batch.hmget('refresh:' + data, 'value', 'result');
            }
            batch.exec((err, replies) => {
                // To SQL
            });
        }
    }
}

class PostgreSQLConnector extends DBConnector {
    constructor(connection_path) {
        super(connection_path);
        this.open();
    }

    open() {
        const vars = this.connection_path_.split(':');
        const host = vars[0];
        const port = Number(vars[1]);
        const database = vars[2];
        this.db_ = new pg.Client({ host: host, port: port, database: database });
        this.db_.connect((err) => {
            if (err) {
                throw err;
            }
            console.log('Connected to database', database);
        });
    }
}

module.exports.RedisConnector = RedisConnector;
module.exports.PostgreSQLConnector = PostgreSQLConnector;


