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
            const data_config = JSON.parse(data_buffer.toString());
            let redis_string = '127.0.0.1:6379'; // default
            if (data_config.redis)
            {
                redis_string = data_config.redis;
            }
            let webapi_string = 'http://localhost:80/ms';
            if (data_config.webapi)
            {
                webapi_string = data_config.webapi;
            }
            let user_password = [null, null];
            if (data_config.weblogin)
            {
                user_password = data_config.weblogin.split(':');      
            }
            const webapi = new WebServiceConnector(webapi_string, user_password[0], user_password[1]);
            const redis_connector = new RedisConnector(redis_string, data_config, webapi);
        }
        catch (e)
        {
            console.error(e);
            process.exit(1);
        }
    }
});

