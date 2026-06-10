#!/usr/bin/env node
// Tenant and session manager.
// 1. Tenant User and password validation.
// 2. Access backend DB.
// 3. Session, mainly token and access time control.
// @date 2023.04.24

const fs = require('node:fs').promises;
const yargs = require('yargs/yargs');
const { hideBin } = require('yargs/helpers');
const fastify = require('fastify')({ logger: true });
const SessionManager = require('./session').SessionManager;
const PostgreSQLConnector = require('./connector').PostgreSQLConnector;

// Default host and port.
let HOST = process.env.HOST || '127.0.0.1';
let PORT = parseInt(process.env.PORT, 10) || 6301;

// Globale interval reference for session cleanup.
let sessionCleanupInterval = null;
const services = {};

async function main() {
    try {
        // 1. 解析命令行参数
        const argv = yargs(hideBin(process.argv))
            .demandOption('f', 'Service config file is required')
            .nargs('f', 1)
            .describe('f', 'Path to service config JSON file')
            .argv;

        const config_str = await fs.readFile(argv.f, 'utf-8');
        const config = JSON.parse(config_str);

        if (config.host) {
            const [host, port] = config.host.split(':');
            if (host) { HOST = host; }
            if (port) { PORT = parseInt(port, 10); }
        }

        if (!config.db) {
            throw new Error('DB connection setting missing in config.');
        }

        // Initialize services.
        services.pg = new PostgreSQLConnector(config.db);
        await services.pg.open();

        services.session = new SessionManager();

        // Register routes. 
        registerRoutes();
        await startServer();

    } catch (err) {
        fastify.log.error(`Fatal startup error: ${err.message}`);
        process.exit(1);
    }
}

function registerRoutes() {
    // Register reusable JSON Schemas that can be referenced across multiple routes for validation and serialization.
    fastify.addSchema({
        $id: 'requestSchema',
        type: 'object',
        required: ['name', 'operation'],
        properties: {
            name: { type: 'string' },
            operation: { type: 'string', pattern: '^(LOGIN|TOUCH|login|touch)$' },
            condition: { type: 'object' }
        }
    });

    fastify.post('/api', {
        schema: { body: { $ref: 'requestSchema' } }
    },
        async (req, reply) => {
            const { operation, condition = {} } = req.body;
            const op = operation.toUpperCase();

            try {
                if (op === 'LOGIN') {
                    const { username, password } = condition;
                    if (!username || !password) {
                        // Standardized error response format for client-side handling.
                        return reply.code(400).send({ message: 'Username or password missing', error: 'INVALID_INPUT' });
                    }

                    const isValid = await services.pg.validateUser(username, '127.0.0.1', password);
                    if (!isValid) {
                        return reply.code(401).send({ message: 'Authentication failed', error: 'INVALID_CREDENTIALS' });
                    }

                    const token = services.session.createSession(username);
                    return reply.code(200).send({ message: 'Login successful', result: { token } });
                }

                if (op === 'TOUCH') {
                    const { token } = condition;
                    if (!token) {
                        return reply.code(400).send({ message: 'Token missing', error: 'INVALID_INPUT' });
                    }

                    const username = services.session.touchSession(token);
                    if (!username) {
                        return reply.code(401).send({ message: 'Session expired or invalid', error: 'SESSION_EXPIRED' });
                    }
                    return reply.code(200).send({ message: 'Session refreshed', result: { username } });
                }

                return reply.code(400).send({ message: 'Unsupported operation', error: 'UNSUPPORTED_OP' });
            } catch (err) {
                req.log.error({ err, operation: op }, 'Route execution failed');
                return reply.code(500).send({ message: 'Internal server error', error: 'SERVER_ERROR' }); // Return a safe, de-sensitized error response.
            }
        });

    // Glabal error handler to catch unhandled exceptions.
    fastify.setErrorHandler((error, request, reply) => {
        request.log.error(error);
        if (!reply.sent) {
            reply.code(500).send({ message: 'Internal server error', error: 'UNHANDLED_EXCEPTION' });
        }
    });
}

async function startServer() {
    // Cleanup expired sessions on a regular interval.
    sessionCleanupInterval = setInterval(async () => {
        try {
            await services.session.cleanupAllSessions();
        } catch (err) {
            fastify.log.error('Session cleanup failed:', err);
        }
    }, 60_000);
    sessionCleanupInterval.unref?.(); // Prevent this interval from stopping the process from exiting.

    const address = await fastify.listen({ port: PORT, host: HOST });
    fastify.log.info(`✅ Tenant running at ${address}`);
}

// Handle the process shutdown gracefully by clearing intervals and DB connections.
const gracefulShutdown = async (signal) => {
    fastify.log.info(`🛑 Received ${signal}. Shutting down gracefully...`);
    if (sessionCleanupInterval) {
        clearInterval(sessionCleanupInterval);
    }

    try {
        if (services.pg) {
            await services.pg.close();
        }
        await fastify.close();
    } catch (err) {
        fastify.log.error('⚠️ Error during shutdown:', err);
        process.exit(1);
    }

    process.exit(0);
};

process.on('SIGINT', () => gracefulShutdown('SIGINT'));
process.on('SIGTERM', () => gracefulShutdown('SIGTERM'));

// 启动入口
main();
