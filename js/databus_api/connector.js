// @date 2023.03.03

// Databus API connector.
// @date 2026.06.08
const Redis = require('ioredis');
const crypto = require('node:crypto');

// 常量定义（保持与原逻辑兼容）
const D_NAMESPACE = { NS_REFRESH: 'refresh:', NS_POLL: 'poll:', NS_REFRESH_TIME: 'time_r:', NS_POLL_TIME: 'time_p:' };
const D_PRIVILEGE = { READ_ONLY: 0, WRITE_ONLY: 1, READ_WRITE: 2 };

class DataInfo {
  constructor(id, name, type, privilege, ratio, value, result, timestamp) {
    this.id = id; this.name = name; this.type = type; this.privilege = privilege;
    this.ratio = ratio; this.value = value; this.result = result; this.timestamp = timestamp;
  }
}

class DataModel {
  constructor() { this.store = new Map(); }
  add(key, value) { this.store.set(key, value); }
  query(key) { return this.store.get(key); }
  has(key) { return this.store.has(key); }
  keys() { return this.store.keys(); }
}

class RedisConnector {
  constructor(redisStr, dataConfig) {
    this.client = null;
    this.model = new DataModel();
    this.config = dataConfig;
    const [host, port] = redisStr.split(':');
    this.redisUrl = { host, port: Number(port) };
  }

  async init() {
    this.client = new Redis(this.redisUrl, { maxRetriesPerRequest: 3, retryDelayOnFailover: 100 });

    // 监听连接状态
    this.client.on('connect', () => console.log('✅ Redis connected'));
    this.client.on('error', (err) => console.error('❌ Redis error:', err));

    if (this.config?.drivers) {
      this._buildModel();
    }
    await this._syncExistingData();
  }

  _buildModel() {
    const tNow = Date.now() / 1000;
    const baseName = this.config.name ?? 'dummy';
    for (const driver of this.config.drivers) {
      for (const node of driver.nodes) {
        for (const data of node.data) {
          const id = `${baseName}.${driver.id}.${node.address}.${data.id}`;
          const ratio = Number.isFinite(Number(data.ratio)) ? Number(data.ratio) : 1.0;

          if (!data.register?.startsWith?.('R', 0) && !data.register?.startsWith?.('W', 0)) continue;

          let privilege = D_PRIVILEGE.READ_ONLY;
          let startIdx = 1;
          if (data.register.startsWith('RW')) { privilege = D_PRIVILEGE.READ_WRITE; startIdx = 2; }
          else if (data.register.startsWith('W')) { privilege = D_PRIVILEGE.WRITE_ONLY; }

          const zoneType = Number(data.register[startIdx]);
          if (isNaN(zoneType) || !Number.isInteger(zoneType)) continue;

          let typeStr = data.register.slice(startIdx + 1, startIdx + 4);
          typeStr = (typeStr.length === 3 && Number.isInteger(Number(typeStr[2]))) ? typeStr.slice(0, 2) : typeStr;  // Remove the trail number
          const typeMap = { DF: 0, WUB: 1, WB: 2, DUB: 3, DB: 4, BB: 5, BT: 6, STR: 7 };
          const dtype = typeMap[typeStr] ?? -1;
          if (dtype === -1) continue;

          this.model.add(id, new DataInfo(id, data.name, dtype, privilege, ratio, 0, '-1', tNow));
        }
      }
    }
  }

  async _syncExistingData() {
    for (const ns of ['refresh', 'poll']) {
      const timeKey = ns === 'refresh' ? D_NAMESPACE.NS_REFRESH_TIME : D_NAMESPACE.NS_POLL_TIME;
      const dataKey = ns === 'refresh' ? D_NAMESPACE.NS_REFRESH : D_NAMESPACE.NS_POLL;
      const existing = await this.client.zrange(timeKey, 0, -1);
      const newIds = new Set(this.model.keys());
      existing.forEach(k => newIds.delete(k.slice(dataKey.length)));

      const pipeline = this.client.pipeline();
      for (const id of newIds) {
        const info = this.model.query(id);
        if (ns === 'poll' && info.privilege === D_PRIVILEGE.READ_ONLY) continue;
        const nsId = `${dataKey}${id}`;
        pipeline.hmset(nsId, { id: info.id, name: info.name, type: info.type, rw: info.privilege, value: info.value, result: info.result, time: info.timestamp });
        pipeline.zadd(timeKey, info.timestamp, nsId);
      }
      if (newIds.size > 0) {
        await pipeline.exec();
      }
    }
  }

  async updateData(namespace, dataList) {
    if (!dataList?.length) {
      return 0;
    }
    const isRefresh = (namespace === 'refresh');
    const keyNs = isRefresh ? D_NAMESPACE.NS_REFRESH : D_NAMESPACE.NS_POLL;
    const timeNs = isRefresh ? D_NAMESPACE.NS_REFRESH_TIME : D_NAMESPACE.NS_POLL_TIME;

    let updated = 0;
    const batchSize = 64;
    for (let i = 0; i < dataList.length; i += batchSize) {
      const chunk = dataList.slice(i, i + batchSize);
      const pipe = this.client.pipeline();
      chunk.forEach(d => {
        const nsId = `${keyNs}${d.id}`;
        pipe.hmset(nsId, { value: d.value, result: d.result, time: d.time });
        pipe.zadd(timeNs, d.time, nsId);
      });
      const res = await pipe.exec();
      updated += res.filter(r => r[1] === 'OK').length;
    }
    return updated;
  }

  async getData(namespace, groupPrefix, idList, timeRange, props, batchNum, batchSize) {
    const isRefresh = namespace === 'refresh';
    const keyNs = isRefresh ? D_NAMESPACE.NS_REFRESH : D_NAMESPACE.NS_POLL;
    const timeNs = isRefresh ? D_NAMESPACE.NS_REFRESH_TIME : D_NAMESPACE.NS_POLL_TIME;

    const [tStart, tEnd] = timeRange?.length === 2 ? timeRange : [Date.now() / 1000 - 5, Date.now() / 1000];
    const fields = props?.length ? props : ['value', 'result', 'time'];

    const candidates = await this.client.zrangebyscore(timeNs, tStart, tEnd);
    const filterIds = idList?.length ? candidates.filter(k => idList.some(id => k.endsWith(`${groupPrefix}.${id}`))) : candidates;

    // 分页支持
    const offset = (batchNum - 1) * batchSize;
    const page = filterIds.slice(offset, offset + batchSize);

    if (!page.length) {
      return { groupName: groupPrefix, table: { id: [], ...Object.fromEntries(fields.map(f => [f, []])) }, total: 0 };
    }

    const pipe = this.client.pipeline();
    page.forEach(id => pipe.hmget(id, fields));
    const results = await pipe.exec();

    const table = { id: [], ...Object.fromEntries(fields.map(f => [f, []])) };
    results.forEach((res, i) => {
      if (res[0]) {
        return table.id.push(page[i].slice(keyNs.length + groupPrefix.length + 1));
      }
      const val = res[1];
      table.id.push(page[i].slice(keyNs.length + groupPrefix.length + 1));
      fields.forEach((f, fi) => table[f].push(val[fi]));
    });

    return { groupName: groupPrefix, table, total: filterIds.length };
  }

  close() {
    return this.client?.quit();
  }
}

module.exports = { RedisConnector, D_NAMESPACE, D_PRIVILEGE };
