#!/usr/bin/env node

// Webapi server.
// 1. Read driver json and create momory model.
// 2. connect redis and synchronize the model.
// 3. Get/Post by the web api.
// 4. Extend the microservice to data bus.
// @date 2023.02.10

const read_file = require('fs').readFile;
const yargs = require('yargs');
const { D_NAMESPACE, D_PRIVILEGE } = require('./connector');
const { threadCpuUsage } = require('process');
const RedisConnector = require('./connector').RedisConnector;
const server = require('fastify')({ logger: true });
const HOST = process.env.HOST || '192.168.2.177';
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
            if (data_config.tenant)
            {
                service_collection['tenant'] = data_config.tenant;
            }
        }
        catch (e) {
            console.error(e);
            process.exit(1);
        }
    }
});


class TouchRequest {
    constructor(service_name, operation, token) {
        this.name = service_name;
        this.operation = operation;
        this.token = token;
    }

    toJSON() {
        return {
            name: this.name,
            operation: this.operation,
            condition: { token: this.token }
        };
    }
}

class LoginRequest {
    constructor(service_name, operation, username, password) {
        this.name = service_name;
        this.operation = operation;
        this.username = username;
        this.password = password;
    }

    toJSON() {
        return {
            name: this.name,
            operation: this.operation,
            condition: {
                username: this.username,
                password: this.password
            }
        }
    }
}

class APIErrorResponse {
    constructor(message, error, status_code) {
        this.message = message;
        this.error = error;
        this.status_code = status_code;
    }

    toJSON() {
        return {
            message: this.message,
            error: this.error,
            statusCode: this.status_code
        }
    }
}

class APIOKResponse {
    constructor(message, result) {
        this.message = message;
        this.result = result;
    }

    toJSON() {
        return {
            message: this.message,
            result: this.result,
            statusCode: '200'
        }
    }
}

// Calls the tenant service's touch().
async function check_id(token)
{
    const touch_req = new TouchRequest('tenant', 'touch', token);
    const data = JSON.stringify(touch_req); 
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'User-Agent': `nodejs/${process.version}`,
            'Content-Encoding': 'gzip',
            'Accept': 'application/json'
        },
        body: data
    }
    try
    {
        const id_check_req = await fetch(service_collection['tenant'], options);
        const payload = await id_check_req.json();
        if (payload.statusCode == '200')
        {
            return payload.result.username;
        }
        else
        {
            return null;
        }
    }
    catch (err)
    {
        return null;
    }
}


// Calls the tenant service's login(), returns the token.
async function login(username, password) {
    const login_req = new LoginRequest('tenant', 'login', username, password);
    const data = JSON.stringify(login_req);
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'User-Agent': `nodejs/${process.version}`,
            'Content-Encoding': 'gzip',
            'Accept': 'application/json'
        },
        body: data
    }
    try {
        const login_body = await fetch(service_collection['tenant'], options);
        const payload = await login_body.json();
        if (payload.statusCode == '200') {

            return payload.result.token;
        }
        else {
            return null;
        }
    }
    catch (err) {
        return null;
    }
}

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

server.post('/message', async (req, reply) => {
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
        if (operation == 'SETDATAR' || operation == 'SETDATAP')
        {
            /* check ID*/
            const token = req.body.token;
            check_result = await check_id(token);
            if (check_result == null)
            {
                return {
                    message: 'Message post error',
                    error: 'Invalid ID',
                    statusCode: '404'
                };
            }
            let namespace = D_NAMESPACE.NS_REFRESH;
            let is_refresh = true;
            if (operation == 'SETDATAP')
            {
                namespace = D_NAMESPACE.NS_POLL;
                is_refresh = false;
            }
            if (!req.body.data || !req.body.data.group_name || 
                !Array.isArray(req.body.data.list))
            {
                throw `Invalid ${operation} content attributes.`;
            }
            const group_name = req.body.data.group_name;
            const service = service_collection['redis'];
            const model = service.data_model();
            const data_list = [];
            const check_set = new Set();
            for (const item of req.body.data.list)
            {
                if (!item.id || !item.value || !item.result || Number.isNaN(Number(item.time)))
                {
                    throw `Invalid ${operation} data item.`;
                }
                const newid = [group_name, item.id].join('.');
                // Distinct id
                if (check_set.has(newid)) 
                {
                    continue;
                }
                // Query data model by new id.
                const data_info = model.query(newid);
                // Exclude undifined and READ_ONLY data.
                if (!data_info || (!is_refresh && data_info.privilege == D_PRIVILEGE.READ_ONLY) ||
                (is_refresh && data_info.privilege == D_PRIVILEGE.WRITE_ONLY))
                {
                    continue; //throw 'Data id does not existed.';
                }
                check_set.add(newid);
                // Shadow copy.
                data_list.push({
                    'id': newid, 'value': item.value, 'result': item.result, 'time': item.time})
            }
            const updated_num = await service.update_data(namespace, data_list);
            return {
                message: `Message post (${operation}) ok`,
                result: {'total': updated_num},
                statusCode: '200'
            };
        }
        /* GETDATA format
        {
            "name": "service_name",
            "operation": "GetData",
            "token": "6ac89607254a437c90c28ccc1c034706",
            "condition": {
                "group_name": "goiot",
                "id_list": ["mfcpfc.1.pv", "mfcpfc.2.pv"],
                "time_range": [1677154222.8210001, 1677154222.8410001],
                "properties": ["value", "result", "timestamp"],
                "batch_size":128,
                "batch_num": 1
            }
        }
        */
        else if (operation == 'GETDATAR' || operation == 'GETDATAP') {
            /* check ID*/
            const token = req.body.token;
            check_result = await check_id(token);
            if (check_result == null) {
                return {
                    message: 'Message post error',
                    error: 'Invalid ID',
                    statusCode: '404'
                };
            }
            let namespace = D_NAMESPACE.NS_REFRESH;
            if (operation == 'GETDATAP') {
                namespace = D_NAMESPACE.NS_POLL;
            }
            const batch_num = Number.parseInt(req.body.condition.batch_num);
            const batch_size = Number.parseInt(req.body.condition.batch_size);
            if (!req.body.condition || !req.body.condition.group_name || 
                !Array.isArray(req.body.condition.id_list) || 
                !Array.isArray(req.body.condition.time_range) ||
                !Array.isArray(req.body.condition.properties) ||
                isNaN(batch_num) || isNaN(batch_size))
            {
                throw `Invalid ${operation} content attributes.`;
            }
            const service = service_collection['redis'];
            const model = service.data_model();
            const group_name = req.body.condition.group_name;
            // Distinct id
            const check_set = new Set(req.body.condition.id_list.map(
                x => [group_name, x].join('.')));
            // Check ids in model
            const id_list = [];
            check_set.forEach(x => {
                if (model.has(x)) {
                    id_list.push(x);
                }
            });
            let result_data = {'group_name': group_name, 'list': [], 'total': 0};  
            // Id matched.  
            if (!(check_set.size > 0 && id_list.length == 0))
            {
                result_data = await service.get_data(namespace, group_name, id_list, 
                    req.body.condition.time_range, req.body.condition.properties, 
                    batch_num, batch_size);
            }
            return {
                    message: `Message post (${operation}) ok`,
                    result: {'data': result_data},
                    statusCode: '200'
                };
        }
        else if (operation == 'LOGIN')
        {
            const username = req.body.condition.username;
            const password = req.body.condition.password;
            const token = await login(username, password);
            if (token == null) {
                const err_response =
                    new APIErrorResponse('Message post(LOGIN) error', 'Login failed', '404');
                return JSON.stringify(err_response);
            }
            else
            {
                const ok_resposne = new APIOKResponse('Message post(LOGIN) ok', {token: token})
                return JSON.stringify(ok_resposne);
            }
        }
        else if (operation == 'TOUCH')
        {
            const token = req.body.condition.token;
            check_result = await check_id(token);
            if (check_result == null)
            {
                const err_response =
                new APIErrorResponse('Message post(TOUCH) error', 'Invalid ID', '404');
                return JSON.stringify(err_response);
            }
            else
            {
                const ok_resposne = new APIOKResponse('Message post(TOUCH) ok', {username: check_result})
                return JSON.stringify(ok_resposne);
            }
        }
        else
        {
            throw 'Unsupported operation.'
        }
    }
    catch (err) {
        const err_response =
            new APIErrorResponse('Message post error', err.message ?? err, '404');
        return JSON.stringify(err_response);
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
