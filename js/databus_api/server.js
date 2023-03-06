#!/usr/bin/env node

// Webapi server.
// 1. Read driver json and create momory model.
// 2. connect redis and synchronize the model.
// 3. Get/Post by the web api.
// 4. Extend the microservice to data bus.
// @date 2023.02.10

const read_file = require('fs').readFile;
const { timeStamp } = require('console');
const yargs = require('yargs');
const RedisConnector = require('./connector').RedisConnector;
const server = require('fastify')({ logger: true });
const HOST = process.env.HOST || '127.0.0.1';
const PORT = process.env.PORT || 6300;
const service_collection = {};

const argv = yargs.demandOption('f').nargs('f', 1).describe('f', 'JSON file to parse').argv;
const file = argv.f;

read_file(argv.f, (err, data_buffer) => {
    if (err) {
        console.error(`ERROR: ${err.message}`);
        process.exit(1);
    }
    else {
        try {
            const data_config = JSON.parse(data_buffer.toString());
            redis_string = '127.0.0.1:6379';
            if (data_config.redis) {
                redis_string = data_config.redis;
            }
            const redis_connector = new RedisConnector(redis_string, data_config);
            service_collection['redis'] = redis_connector; // register service.
        }
        catch (e) {
            console.error(e);
            process.exit(1);
        }
    }
});

server.get('/message', async (req, reply) => {
    console.log(`worker request pid=${process.pid}`);
    return {
        message: 'Databus message api need right request format.',
        error: 'Wrong request',
        statusCode: '404'
    };
});

server.post('/message', async (req, reply) => {
    console.log(`worker request pid=${process.pid}`);
    try 
    {
        if (req.headers['content-type'] != 'application/json') 
        {
            throw 'Invalid content-type';
        }
        if (!req.body || !req.body.name || !req.body.token || !req.body.operation)
        {
            throw 'Invalid content';
        }
        const token = req.body.token;
        const operation = req.body.operation.toUpperCase();
        if (operation == 'SETDATA')
        {
            if (!req.body.data || !req.body.data.group_name || !req.body.data.group_name ||
                !Array.isArray(req.body.data.list))
            {
                throw `Invalid ${operation} content attributes.`;
            }
            const group_name = req.body.data.group_name;
            const service = service_collection['redis'];
            const model = service.data_model();
            const data_list = [];
            for (const item of req.body.data.list)
            {
                if (!item.id || !item.value || !item.result || isNaN(Number.parseFloat(item.timestamp)))
                {
                    throw `Invalid ${operation} data item.`;
                }
                const newid = [group_name, item.id].join('.');
                // Query data model by new id.
                if (!model.has(newid))
                {
                    throw 'Data id does not existed.';
                }
                // Shadow copy.
                data_list.push({
                    'id': newid, 'value': item.value, 'result': item.result, 'timestamp': item.timestamp})
            }
            const reply = await service.update_data(data_list);
            return {
                message: `Message post (${operation}) ok`,
                data: {'req_num': data_list.length, 'ok_num': reply},
                statusCode: '200'
            };
        }
        throw 'Unsupported operation.'
    }
    catch (err) {
        return {
            message: 'Message post error',
            error: err,
            statusCode: '404'
        };
    }
});

const start = async () => {
    try {
        await server.listen({ port: PORT, host: HOST }, () => {
            console.log(`Databus api running at http://${HOST}:${PORT}`);
        })
    }
    catch (err) {
        server.log.error(err)
        process.exit(1)
    }
}

start();
