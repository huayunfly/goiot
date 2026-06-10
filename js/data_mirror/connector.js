const { fetch } = require('undici'); // Node 18+ 可替换为 globalThis.fetch
const Redis = require('ioredis');

const D_NAMESPACE = {
  NS_REFRESH: 'refresh:',
  NS_POLL: 'poll:',
  NS_REFRESH_TIME: 'time_r:',
  NS_POLL_TIME: 'time_p:'
};

/**
 * 现代化 HTTP 客户端
 * 特性：超时控制、指数退避重试、Token 自动续期、适配新 Databus 响应格式
 */
class DatabusClient {
  constructor(options = {}) {
    this.baseUrl = options.baseUrl.replace(/\/+$/, '');
    this.username = options.username;
    this.password = options.password;
    this.token = null;
    this.timeout = options.httpTimeout || 5000;
    this.maxRetries = options.maxRetries || 3;
  }

  async _request(payload) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), this.timeout);

    for (let attempt = 0; attempt <= this.maxRetries; attempt++) {
      try {
        const res = await fetch(`${this.baseUrl}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
          body: JSON.stringify(payload),
          signal: controller.signal
        });
        clearTimeout(timer);

        const data = await res.json().catch(() => ({}));
        if (!res.ok) {
          const errMsg = data.message || `HTTP ${res.status}`;
          if (res.status === 401 && payload.operation !== 'LOGIN') {
            this.token = null; // 触发重新认证
          }
          throw new Error(errMsg);
        }
        return data;
      } catch (err) {
        clearTimeout(timer);
        if (err.name === 'AbortError' || attempt === this.maxRetries) {
            throw err;
        }
        await new Promise(r => setTimeout(r, 1000 * (attempt + 1))); // 指数退避
      }
    }
  }

  async login() {
    if (!this.username || !this.password) return false;
    try {
      const res = await this._request({
        name: 'tenant',
        operation: 'LOGIN',
        condition: { username: this.username, password: this.password }
      });
      this.token = res.result?.token || null;
      return !!this.token;
    } catch (err) {
      console.error('🔐 登录失败:', err.message);
      return false;
    }
  }

  async touch() {
    if (!this.token) return false;
    try {
      const res = await this._request({
        name: 'tenant',
        operation: 'TOUCH',
        condition: { token: this.token }
      });
      return !!res.result?.username;
    } catch (err) {
      console.warn('🔄 Token 续期失败:', err.message);
      this.token = null;
      return false;
    }
  }

  async postData(name, operation, payload) {
    if (!this.token) {
      const success = await this.login();
      if (!success) {
        throw new Error('认证失败，无法发起数据请求');
      }
    }
    return this._request({name, operation, token: this.token, ...payload });
  }
}

/**
 * Redis 同步管理器
 * 特性：安全定时器、Pipeline 批量操作、防并发锁、优雅关闭
 */
class RedisSyncManager {
  constructor(redisUrl, dataConfig, clientOptions) {
    this.redisUrl = redisUrl;
    this.config = dataConfig;
    this.client = new DatabusClient(clientOptions);
    this.redis = null;
    this.model = [];
    this.modelWrite = [];
    this.timers = [];
    this.isRunning = false;
    this.syncLock = false; // 防定时器重叠执行
  }

  async init() {
    const [host, port] = this.redisUrl.split(':');
    this.redis = new Redis({ host, port: Number(port), maxRetriesPerRequest: 3 });
    
    this.redis.on('error', err => console.error('❌ Redis 错误:', err));
    this.redis.on('ready', () => {
      console.log('✅ Redis 连接就绪');
      this._buildModel();
      this._startSyncLoops();
    });

    if (this.config.weblogin) {
      await this.client.login();
    }
  }

  _buildModel() {
    const groupName = this.config.name || 'default';
    for (const driver of (this.config.drivers || [])) {
      const ids = [];
      const writeIds = [];
      for (const node of driver.nodes) {
        for (const data of node.data) {
          const id = `${driver.id}.${node.address}.${data.id}`;
          if (data.register.toUpperCase().startsWith('R')) {
            ids.push(id);
          }
          if (data.register.toUpperCase().match(/^R?W/)) {
            writeIds.push(id);
          }
        }
      }
      this.model.push({ group: groupName, driver: driver.id, ids: ids });
      this.modelWrite.push({ group: groupName, driver: driver.id, ids: writeIds });
    }
    console.log(`✅ 模型构建完成: ${this.model.length} 读组, ${this.modelWrite.length} 写组`);
  }

  _startSyncLoops() {
    this.isRunning = true;
    const refreshInt = setInterval(() => this._syncRefresh().catch(console.error), this.client.options?.refreshInterval || 3000);
    const pollInt = setInterval(() => this._syncPoll().catch(console.error), this.client.options?.pollInterval || 1000);
    const checkInt = setInterval(() => this._checkConnection().catch(console.error), this.client.options?.checkInterval || 10000);
    this.timers.push(refreshInt, pollInt, checkInt);
  }

  async stop() {
    console.log('🛑 安全关闭 Datamirror...');
    this.isRunning = false;
    this.timers.forEach(t => clearInterval(t));
    await this.redis?.quit();
    console.log('✅ 所有连接已释放，退出进程。');
    process.exit(0);
  }

  async _syncRefresh() {
    if (!this.isRunning || !this.client.token || this.syncLock) {
        return;
    }
    this.syncLock = true;
    try {
      for (const table of this.model) {
        const pipeline = this.redis.pipeline();
        for (const id of table.ids) {
          pipeline.hmget(`${D_NAMESPACE.NS_REFRESH}${id}`, 'value', 'result', 'time');
        }
        const results = await pipeline.exec();
        
        const validData = results
          .map(([err, vals], i) => err ? null : { 
            id: table.ids[i], 
            value: vals[0], 
            result: vals[1], 
            time: Number(vals[2]) 
          })
          .filter(d => d && d.time && d.time > (Date.now() / 1000 - 15));

        if (validData.length > 0) {
          await this.client.postData('databus', 'SETDATAR', {
            data: {
              groupName: table.group,
              table: {
                id: validData.map(d => d.id),
                value: validData.map(d => d.value),
                result: validData.map(d => d.result),
                time: validData.map(d => d.time)
              }
            }
          });
        }
      }
    } catch (err) {
      console.error('📤 Refresh 同步失败:', err.message);
    } finally {
      this.syncLock = false;
    }
  }

  async _syncPoll() {
    if (!this.isRunning || !this.client.token || this.syncLock) {
      return;
    }
    this.syncLock = true;
    try {
      const batchSize = this.client.options?.batchSize || 128;
      for (const table of this.modelWrite) {
        for (let i = 0; i < table.ids.length; i += batchSize) {
          const batchIds = table.ids.slice(i, i + batchSize);
          const timeRange = [Date.now() / 1000 - 15, Date.now() / 1000];
          
          const res = await this.client.postData('databus', 'GETDATAP', {
            condition: {
              groupName: table.group,
              idList: batchIds,
              timeRange: timeRange,
              properties: ['value', 'result', 'time'],
              batchNum: 1,
              batchSize: batchSize
            }
          });

          if (res?.data?.table?.id?.length) {
            const t = res.data.table;
            if (t.id.length !== t.value.length || t.id.length !== t.result.length || t.id.length !== t.time.length) {
              console.warn('⚠️ 数据表字段长度不一致，跳过批次');
              continue;
            }
            
            const pipe = this.redis.pipeline();
            t.id.forEach((id, idx) => {
              const time = Number(t.time[idx]);
              if (!Number.isNaN(time)) {
                const key = `${D_NAMESPACE.NS_POLL}${id}`;
                pipe.hmset(key, { value: t.value[idx], result: t.result[idx], time });
                pipe.zadd(D_NAMESPACE.NS_POLL_TIME, time, key);
              }
            });
            await pipe.exec();
          }
        }
      }
    } catch (err) {
      console.error('📥 Poll 拉取失败:', err.message);
    } finally {
      this.syncLock = false;
    }
  }

  async _checkConnection() {
    if (!this.isRunning) return;
    const isValid = await this.client.touch();
    if (!isValid && this.config.weblogin) {
      console.log('🔄 会话过期，重新登录...');
      await this.client.login();
    }
  }
}

module.exports = { DatabusClient, RedisSyncManager };
