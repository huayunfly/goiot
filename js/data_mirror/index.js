// Synchronize data from redis to data_webapi.
// @date 2020.12.26

// RESTful API practice.
// @date 2026.06.09

const fs = require('node:fs').promises;
const yargs = require('yargs/yargs');
const { hideBin } = require('yargs/helpers');
const { RedisSyncManager } = require('./connector');

async function main() {
  const argv = yargs(hideBin(process.argv))
    .option('f', {
      alias: 'file',
      describe: '配置文件路径',
      type: 'string',
      demandOption: true
    })
    .option('host', {
      alias: 'H',
      describe: '覆盖 Redis 地址',
      type: 'string',
      default: process.env.REDIS_HOST
    })
    .option('webapi', {
      alias: 'W',
      describe: '覆盖 Databus API 地址',
      type: 'string',
      default: process.env.WEBAPI_URL
    })
    .parse();

  const config = JSON.parse(await fs.readFile(argv.f, 'utf-8'));
  
  // 优先级：命令行 > 环境变量 > 配置文件 > 默认值
  const redisUrl = argv.host || config.redis || '127.0.0.1:6379';
  const webapiUrl = argv.webapi || config.webapi || 'http://localhost:6300/message';
  const [username, password] = (config.weblogin || ':').split(':');

  console.log(`✅ Config loaded from: ${argv.f}`);
  console.log(`✅ Redis: ${redisUrl} | Databus: ${webapiUrl}`);

  const syncManager = new RedisSyncManager(redisUrl, config, {
    baseUrl: webapiUrl,
    username: username || undefined,
    password: password || undefined,
    refreshInterval: config.refreshInterval || 3000,
    pollInterval: config.pollInterval || 1000,
    checkInterval: config.checkInterval || 10000,
    batchSize: config.batchSize || 128,
    httpTimeout: config.httpTimeout || 5000,
    maxRetries: config.maxRetries || 3
  });

  await syncManager.init();
  console.log('✅ Datamirror 已启动。按 Ctrl+C 安全退出。');

  process.on('SIGINT', () => syncManager.stop());
  process.on('SIGTERM', () => syncManager.stop());
}

main().catch(err => {
  console.error('⛔ 启动失败:', err);
  process.exit(1);
});

