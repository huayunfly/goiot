/// npm taobao mirror: npm install -gd express --registry=http://registry.npm.taobao.org
/// Or setting the global: npm config set registry http://registry.npm.taobao.org

const read_file = require('fs').readFile;
const yargs = require('yargs');
const RedisConnector = require('./db_connector').RedisConnector;
const PostgreSQLConnector = require('./db_connector').PostgreSQLConnector;

const argv = yargs.demandOption('f').nargs('f', 1).describe('f', 'JSON file to parse').argv;
const file = argv.f;

read_file(argv.f, (err, data_buffer) => {
    if (err)
    {
        console.error(`ERROR: ${err.message}`);
        process.exit(1);
    }
    else
    {
        try
        {
            const data_config = JSON.parse(data_buffer.toString());
            const postgresql = new PostgreSQLConnector('127.0.0.1:5432:goiot:postgres:hello@123');
            const redis_connector = new RedisConnector('127.0.0.1:6379', data_config, postgresql);
        }
        catch (e)
        {
            console.error(e);
            process.exit(1);
        }
    }
});

