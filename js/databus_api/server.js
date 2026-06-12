#!/usr/bin/env node

// Webapi server.
// 1. Read driver json and create momory model.
// 2. connect redis and synchronize the model.
// 3. Get/Post by the web api.
// 4. Extend the microservice to data bus.
// @date 2023.02.10


// Databus API Server
// Modern Fastify architecture with schema validation, auth hooks, and graceful shutdown.
// @date 2026.06.08 

const fs = require('node:fs').promises;
const yargs = require('yargs/yargs');
const { hideBin } = require('yargs/helpers');
const fastify = require('fastify')({ logger: { level: 'info' } });

const { RedisConnector } = require('./connector');
const TenantClient = require('./tenant-client');

const HOST = process.env.HOST || '0.0.0.0';
const PORT = parseInt(process.env.PORT, 10) || 6300;

let redisService;
let tenantClient;

// ================= 1. 统一请求体校验 Schema =================
const messageSchema = {
    type: 'object',
    required: ['name', 'operation'],
    properties: {
        operation: {
            type: 'string',
            enum: ['LOGIN', 'TOUCH', 'SETDATAR', 'SETDATAP', 'GETDATAR', 'GETDATAP', 'login', 'touch', 'setdatar', 'setdatap', 'getdatar', 'getdatap']
        },
        token: { type: 'string' },
        condition: { type: 'object' },
        data: { type: 'object' }
    },
    additionalProperties: false // 拦截未知字段，防注入攻击
};

// ================= 2. 路由定义（解决重复注册报错） =================
fastify.post('/message', {
    schema: { body: messageSchema },
    logLevel: 'info'
}, async (req, reply) => {
    const op = req.body.operation.toUpperCase();

    // 🔹 分支 A：认证类操作（跳过 Token 校验，直连租户服务）
    if (op === 'LOGIN' || op === 'TOUCH') {
        if (!tenantClient) {
            return reply.code(503).send({ message: 'Tenant service not configured', error: 'TENANT_NOT_CONFIGURED' });
        }
        try {
            if (op === 'LOGIN') {
                const { username, password } = req.body.condition || {};
                if (!username || !password) {
                    return reply.code(400).send({ message: 'Username or password required', error: 'MISSING_CREDENTIALS' });
                }
                const token = await tenantClient.login(username, password);
                if (!token) {
                    return reply.code(401).send({ message: 'Username or password incorrect', error: 'AUTH_FAILED' });
                }
                return reply.code(200).send({ message: 'LOGIN ok', result: { token } });
            }

            if (op === 'TOUCH') {
                const token = req.body.condition?.token;
                if (!token) {
                    return reply.code(400).send({ message: 'Token required', error: 'MISSING_TOKEN' });
                }
                const username = await tenantClient.touch(token);
                if (!username) {
                    return reply.code(401).send({ message: 'Session expired or invalid token', error: 'SESSION_EXPIRED' });
                }
                return reply.code(200).send({ message: 'TOUCH ok', result: { username } });
            }
        } catch (err) {
            req.log.error({ err, operation: op }, 'Tenant service error');
            return reply.code(503).send({ message: 'Tenant service unavailable', error: 'TENANT_UNAVAILABLE' });
        }
    }

    // 分支 B：数据操作类（强制 Token 鉴权）
    if (!req.body.token) {
        return reply.code(401).send({ message: 'Token required', error: 'MISSING_TOKEN'});
    }

    try {
        if (!tenantClient) {
            throw new Error('Tenant client not initialized');
        }
        const username = await tenantClient.touch(req.body.token);
        if (!username) {
            return reply.code(401).send({ message: 'Invalid or expired token', error: 'INVALID_TOKEN'});
        }
        req.username = username; // 注入请求上下文
    } catch (err) {
        req.log.error({ err }, 'Token validation failed');
        return reply.code(503).send({ message: 'Tenant service unavailable', error: 'TENANT_UNAVAILABLE' });
    }

    // 分支 C：核心数据读写逻辑
    try {
        if (op.startsWith('SETDATA')) {
            const { groupName, table } = req.body.condition || {};
            const namespace = (op === 'SETDATAP' ? 'poll' : 'refresh');

            if (!groupName || !table || !Array.isArray(table.id) || !Array.isArray(table.value) ||
                !Array.isArray(table.result) || !Array.isArray(table.time)) {
                return reply.code(400).send({ message: 'Invalid data table structure',  error: 'INVALID_PAYLOAD'});
            }
            if (table.id.length !== table.value.length || table.id.length !== table.result.length || table.id.length !== table.time.length) {
                return reply.code(400).send({ message: 'Data table arrays have inconsistent lengths', code: 'ARRAY_MISMATCH'});
            }

            const dataList = table.id.map((id, i) => ({
                id: `${groupName}.${id}`,
                value: table.value[i],
                result: table.result[i],
                time: table.time[i]
            }));

            const count = await redisService.updateData(namespace, dataList);
            return reply.code(200).send({ message: `SETDATA ${namespace} ok`, result: { updated: `${count}` } });
        }

        if (op.startsWith('GETDATA')) {
            const condition = req.body.condition || {};
            const { groupName, idList, timeRange, properties, batchSize, batchNum } = condition;
            const namespace = (op === 'GETDATAP' ? 'poll' : 'refresh');

            if (!groupName || !Array.isArray(idList) || idList.length === 0) {
                return reply.code(400).send({ message: 'Missing groupName or idList', error: 'INVALID_PAYLOAD' });
            }

            const data = await redisService.getData(
                namespace,
                groupName,
                idList,
                timeRange,
                properties,
                batchNum || 1,
                batchSize || 128
            );
            return reply.code(200).send({ message: `GETDATA ${namespace} ok`, result: data });
        }
    } catch (err) {
        req.log.error({ err, operation: op }, '数据操作执行失败');
        return reply.code(500).send({ error: { code: 'INTERNAL_ERROR', message: '业务处理失败' } });
    }
});

// ================= 3. 健康检查端点（现代 API 标配） =================
fastify.get('/health', async () => ({
    message: 'Databus API is healthy',
    result: {
        status: 'ok',
        timestamp: Date.now(),
        redis: redisService ? 'connected' : 'uninitialized',
        tenant: tenantClient ? 'configured' : 'unconfigured'
    }
}));

// ================= 4. 全局错误处理器 =================
fastify.setErrorHandler((err, req, reply) => {
    req.log.error(err);
    if (!reply.sent) {
        const statusCode = err.statusCode || 500;
        reply.code(statusCode).send({
            message: process.env.NODE_ENV === 'production' ? 'Internal Server Error' : err.message,
            error: err.code || 'SERVER_ERROR'
        });
    }
});

// ================= 5. 优雅关闭机制 =================
async function closeServer() {
    fastify.log.info('🛑 Graceful shutdown initiated...');
    try {
        await fastify.close();
        await redisService?.close();
        fastify.log.info('✅ Server closed successfully.');
        process.exit(0);
    } catch (err) {
        fastify.log.error('❌ Failed to close server:', err);
        process.exit(1);
    }
}

process.on('SIGINT', closeServer);
process.on('SIGTERM', closeServer);

// ================= 6. 启动流程 =================
async function main() {
    try {
        const argv = yargs(hideBin(process.argv))
            .demandOption('f', 'Service config file is required')
            .nargs('f', 1)
            .describe('f', 'Path to service config JSON file')
            .argv;

        const config_str = await fs.readFile(argv.f, 'utf-8');
        const config = JSON.parse(config_str);

        // 初始化 Redis 连接器
        const redisHost = config.redis || '127.0.0.1:6379';
        redisService = new RedisConnector(redisHost, config);
        await redisService.init();
        fastify.log.info('✅ Redis connector initialized');

        // 初始化租户客户端
        if (config.tenant) {
            tenantClient = new TenantClient(config.tenant);
            fastify.log.info('✅ Tenant client initialized');
        } else {
            fastify.log.warn('⚠️ Tenant service URL not configured. LOGIN/TOUCH operations will return 503.');
        }

        await fastify.listen({ port: PORT, host: HOST });
        fastify.log.info(`Databus API running at http://${HOST}:${PORT}`);
    } catch (err) {
        fastify.log.error('Fatal startup error:', err);
        process.exit(1);
    }
}

main();

