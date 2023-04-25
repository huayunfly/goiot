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
        this.db_ = new pg.Client({
            host: host, port: port,
            database: database, user: user, password: password
        });
        this.db_.connect((err) => {
            if (err) {
                throw err;
            }
            console.log('PostgreSQL client connected to database', database);
        });
    }

    close() {
        if (!this.db_) {
            this.db_.end();
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
 ${data_array.map(n => '\'' + n.toString() + '\'').join(',')}, ${Date.now() / 1000}
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

module.exports.PostgreSQLConnector = PostgreSQLConnector;


