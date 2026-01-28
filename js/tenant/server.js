#!/usr/bin/env node

// Tenant and session manager.
// 1. Tenant User and password validation.
// 2. Access backend DB.
// 3. Session, mainly token and access time control.
// @date 2023.04.24

const assert = require('node:assert').strict;
const read_file = require('fs').readFile;
const yargs = require('yargs');
const SessionMagager = require('./session').SessionMagager;
const PostgreSQLConnector = require('./connector').PostgreSQLConnector;
const server = require('fastify')({ logger: true });
let HOST = process.env.HOST;
let PORT = process.env.PORT;
const service_collection = {};

const argv = yargs.demandOption('f').nargs('f', 1).describe('f', 'service config file to parse').argv;
const file = argv.f;

read_file(argv.f, (err, data_buffer) => {
    if (err) 
    {
        console.error(`ERROR: ${err.message}`);
        process.exit(1);
    }
    else 
    {
        try {
            const service_config = JSON.parse(data_buffer.toString());
            host_string = '127.0.0.1:6301';
            if (service_config.host) 
            {
                host_string = service_config.host;
            }
            host_setting = host_string.split(':');
            HOST = host_setting[0];
            if (host_setting.length > 0)
            {
                PORT = host_setting[1];
            }
            if (service_config.db)
            {
                const pg_connector = new PostgreSQLConnector(service_config.db);
                service_collection['pg'] = pg_connector; // register service.
            }
            else
            {
                throw 'DB connection setting missing.';
            }
            service_collection['session'] = new SessionMagager();   
            start(); // service start
        }
        catch (e) {
            console.error(e);
            process.exit(1);
        }
    }
});

server.get('/message', async (req, reply) => {
    console.log(`worker request pid=${process.pid}`);
    try
    {
        throw 'Unsupported operation.';
    }
    catch(err)
    {
        return {
            message: 'Message put error',
            error: err,
            statusCode: '404'
        };
    }
});


server.post('/api', async (req, reply) => {
    console.log(`worker request pid=${process.pid}`);
    try 
    {
        if (req.headers['content-type'] != 'application/json') 
        {
            throw 'Invalid content-type';
        }
        if (!req.body || !req.body.name || !req.body.operation)
        {
            throw 'Invalid content';
        }
        const operation = req.body.operation.toUpperCase();
        if (operation == 'LOGIN')
        {
            const username = req.body.condition?.username;
            const password = req.body.condition?.password;
            if (!username || !password)
            {
                throw 'User name or password missing';
            }
            const host = '127.0.0.1';
            const ok = await service_collection['pg'].validate_user(username, host, password);
            if (!ok)
            {
                return {
                    message: `Api post (${operation}) failed`,
                    error: 'Username or password error',
                    statusCode: '400'
                };
            }
            const s_id = service_collection['session'].create_session(username); 
            return {
                message: `Api post (${operation}) ok`,
                result: {'token': `${s_id}`},
                statusCode: '200'
            };
        }
        else if (operation == 'TOUCH') 
        {
            const token = req.body.condition?.token;
            if (!token)
            {
                throw 'Token missing';
            }
            const username = service_collection['session'].touch_session(token);
            if (username == null)
            {
                return {
                    message: `Api post (${operation}) failed`,
                    error: 'Session expired or not existed',
                    statusCode: '404'
                };
            }  
            return {
                message: `Api post (${operation}) ok`,
                result: {'username': `${username}`},
                statusCode: '200'
            };
        }
        throw 'Unsupported operation.'
    }
    catch (err) {
        return {
            message: 'Message post error',
            error: err.message??err,
            statusCode: '404'
        };
    }
});

const start = async () => 
{
    try {
        await server.listen({ port: PORT, host: HOST }, () => {
            console.log(`Tenant running at http://${HOST}:${PORT}`);
        });
        setInterval(() => {
            service_collection['session'].cleanup_all_sessions();
        }, 60_000);
    }
    catch (err) 
    {
        server.log.error(err)
        process.exit(1)
    }
}

function test() 
{
    const guid = service_collection['session'].create_session();
    service_collection['session'].set_session_data(guid, 'hua', 44);
    assert.equal(service_collection['session'].get_session_data(guid, 'hua'), 44);
    service_collection['session'].cleanup_all_sessions();
}
