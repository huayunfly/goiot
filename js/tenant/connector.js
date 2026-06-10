const pg = require('pg');
const crypto = require('crypto');

async function digestMessage(message) {
    return await crypto.createHash('sha256').update(message).digest('base64');
}

class DBConnector {
    constructor(connectionPath) {
        this.connectionPath_ = connectionPath;
        this.db_ = null;
    }
    async open() { throw new Error('Method open() not implemented'); }
    async close() { throw new Error('Method close() not implemented'); }
}

class PostgreSQLConnector extends DBConnector {
    constructor(connectionPath) {
        super(connectionPath);
        this.tableCreated_ = false;
        this.reconnecting_ = false; // Flag to prevent multiple simultaneous reconnection attempts.
    }

    async open() {
        const vars = this.connectionPath_.split(':');
        if (vars.length < 5) throw new Error('Invalid connection_path format: host:port:db:user:password');

        // pg.Client is a single connection, not suitable for production use.
        // this.db_ = new pg.Client({
        //     host: vars[0],
        //     port: Number(vars[1]),
        //     database: vars[2],
        //     user: vars[3],
        //     password: vars[4]
        // });

        // pg.Pool is used in production.
        this.db_ = new pg.Pool({
            connectionString: `postgres://${vars[3]}:${vars[4]}@${vars[0]}:${vars[1]}/${vars[2]}`,
            max: 10, connectionTimeoutMillis: 2000, idleTimeoutMillis: 30000
        });

        try {
            await this.db_.connect(); // pg returns a promise.
            console.log(`✅ PostgreSQL client connected to '${vars[2]}'`);
        } catch (err) {
            console.error('❌ Failed to connect to PostgreSQL:', err);
            throw err; // Throw to the upper layer to handle connection failure (e.g., retry logic or failover)
        }
    }

    async close() {
        if (this.db_) {
            try {
                await this.db_.close(); // pg Pool's close method returns a promise.
                console.log('✅ PostgreSQL client closed.');
            } catch (err) {
                console.error('⚠️ Error closing DB connection:', err);
            }
        }
    }

async validateUser(user, host, password) {
        let retries = 2; // 允许重试 2 次（针对瞬态网络故障）
        
        while (retries >= 0) {
            try {
                // 1. 状态守卫：连接失效时触发安全重连
                if (!this.db_ || ['disconnected', 'closed', 'ended', 'idle'].includes(this.db_.state)) {
                    await this.ensureConnection();
                }

                const auth_string = await digestMessage(password);
                const result = await this.db_.query(
                    `SELECT COUNT(*) FROM public."user" WHERE "User"=$1 AND "Host"=$2 AND "AuthString"=$3`,
                    [user, host, auth_string]
                );
                return Number.parseInt(result.rows[0].count, 10) > 0;
            } catch (err) {
                console.error(`[DB Error] validateUser retry ${2-retries}:`, err.code, err.message);
                retries--;

                // 2. 非瞬态错误直接终止（如 SQL 语法、权限拒绝、认证失败）
                if (!['ECONNREFUSED', 'ETIMEDOUT', 'ECONNRESET', '57P01', '53300'].includes(err.code)) {
                    return false;
                }

                // 3. 瞬态网络错误等待后重试
                if (retries >= 0) {
                    await new Promise(r => setTimeout(r, 300 * (2 - retries))); // 简单退避
                }
            }
        }
        return false; // Fail-Closed：所有异常最终返回 false
    }

    /** 安全重连守卫（防并发 + 安全关闭 + 异常隔离） */
    async ensureConnection() {
        if (this.reconnecting_) return; // Already connected, skip the attempt.
        this.reconnecting_ = true;
        
        try {
            if (this.db_) {
                try {
                    await this.db_.close(); // Close existing connection.
                } catch (closeErr) {
                    console.warn('⚠️ Failed to close stale connection, forcing recreate:', closeErr.code);
                }
                this.db_ = null;
            }
            await this.open(); // 建立新连接
        } catch (openErr) {
            console.error('❌ Connection recovery failed:', openErr.message);
            // 不向上抛出，交由上层 retries 控制流
        } finally {
            this.reconnecting_ = false; // Release the lock.
        }
    }
}

module.exports = { PostgreSQLConnector };
