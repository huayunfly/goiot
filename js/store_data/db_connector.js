// https://github.com/NodeRedis/node-redis

const redis = require('redis');
const pg = require('pg');
const assert = require('assert');

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
    constructor(connection_path, data_config, sqldb) {
        super(connection_path);
        this.data_config_ = data_config;
        this.keep_running_ = true;
        this.model_ = [];
        this.sqldb_ = sqldb;
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
            this.model_.push({driver_id: driver_id, data_array: data_array });
        }
        this.sqldb_.create_tables(this.model_);
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
                obj.sqldb_.insert(table_name, table.data_array, replies.map(n => Number(n[1]) == 0 ? n[0] : 'NULL'));
            });
        }
    }
}

class PostgreSQLConnector extends DBConnector {
    constructor(connection_path) {
        super(connection_path);
        this.table_created_ = false;
        this.open();
    }

    // Connect to DB.
    open() {
        const vars = this.connection_path_.split(':');
        const host = vars[0];
        const port = Number(vars[1]);
        const database = vars[2];
        const user = vars[3];
        const password = vars[4];
        this.db_ = new pg.Client({ host: host, port: port, 
            database: database, user: user, password: password });
        this.db_.connect((err) => {
            if (err) {
                throw err;
            }
            console.log('PostgreSQL client connected to database', database);
        });
    }

    close() {
        if (!this.db_) {
            this.db_.close();
        }
    }

    // Create DB tables if not exist.
    create_tables(model) {
        if (!model) {
            throw 'Model is null.';
        }  
        // Create table with special column name wrapped with "", like "mfcpfc.4.pv"    
        for (const item of model) {
            this.db_.query(`
 CREATE TABLE IF NOT EXISTS ${item.driver_id}
 (id SERIAL primary key, ${item.data_array.map(n => '"' + n + '"' + ' TEXT').join(',')}, 
 time DOUBLE PRECISION, createtime TIMESTAMP WITH TIME ZONE not null default localtimestamp(0));
 `, (err, result) => {
                if (err) {
                    throw err;
                }
                console.log(`Created table "${item.driver_id}"`);
            });
        }
        this.table_created_ = true;
    }

    // Insert a new record.
    insert(table, names, data_array) {
        if (!this.table_created_) {
            console.log(`Table "${table}" is not existed.`);
            return;
        }   
        // Insert table with special column name wrapped with "", like "mfcpfc.4.pv"     
        this.db_.query(`
 INSERT INTO ${table} (${names.map(n => '"' + n + '"').join(',')}, time) VALUES (
 ${data_array.map(n => '\'' + n.toString() + '\'').join(',')}, ${Date.now()/1000}
 ) RETURNING id;
`, (err, result) => {
            if (err) {
                throw err;
            }
            const id = result.rows[0].id;
            console.log('Inserted row with id %s', id);
        });
    }
}

module.exports.RedisConnector = RedisConnector;
module.exports.PostgreSQLConnector = PostgreSQLConnector;


