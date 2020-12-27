// Synchronize data from redis to data_webapi.
// @date 2020.12.26

const read_file = require('fs').readFile;
const yargs = require('yargs');
const RedisConnector = require('./connector').RedisConnector;
const WebServiceConnector = require('./connector').WebServiceConnector;

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
            //const api_address = 'http://localhost/goiot/data/webapi';
            const webapi = new WebServiceConnector('localhost', 80);
            const data_config = JSON.parse(data_buffer.toString());
            redis_string = '127.0.0.1:6379';
            if (data_config.redis)
            {
                redis_string = data_config.redis;
            }
            const redis_connector = new RedisConnector(redis_string, data_config, webapi);
        }
        catch (e)
        {
            console.error(e);
            process.exit(1);
        }
    }
});

