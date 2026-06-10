const { fetch } = require('undici'); // Node 18+ 原生，或 npm i undici

class TenantClient {
  constructor(baseUrl) {
    this.baseUrl = baseUrl.replace(/\/+$/, '');
    this.timeout = 3000; // 3s 防雪崩
  }

  async _request(body) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), this.timeout);
    try {
      const res = await fetch(`${this.baseUrl}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
        body: JSON.stringify(body),
        signal: controller.signal
      });
      if (res.status === 401 || res.status === 200) {
        return await res.json()
      }
      else {
        throw new Error(`HTTP ${res.status}`);
      }
    } finally {
      clearTimeout(timer);
    }
  }

  async login(username, password) {
    const payload = { name: 'tenant', operation: 'LOGIN', condition: { username, password } };
    const res = await this._request(payload);
    return res?.result?.token ?? null;
  }

  async touch(token) {
    const payload = { name: 'tenant', operation: 'TOUCH', condition: { token } };
    const res = await this._request(payload);
    return res?.result?.username ?? null;
  }
}

module.exports = TenantClient;
