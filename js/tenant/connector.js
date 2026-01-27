const pg = require('pg');
const assert = require('assert');
const crypto = require('crypto')

async function digest_message(message) 
{
    return crypto.createHash("SHA256").update(message).digest("base64");
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
            console.log(`PostgreSQL client connected to database '${database}'`);
        });
    }

    close() {
        if (!this.db_) {
            this.db_.end();
        }
    }

    // Validate user by password, return true if OK. 
   async validate_user(user, host, password)
    {   
        const auth_string = await digest_message(password);
        const result = await this.db_.query(`
        SELECT COUNT(*)
        FROM public."user" WHERE "User"='${user}' and "Host"='${host}' and "AuthString"='${auth_string}';
         `);
        if (result === undefined || Number.parseInt(result.rows[0].count) == 0)
        {
            return false;
        }
        return true;
    }
}

module.exports.PostgreSQLConnector = PostgreSQLConnector;


