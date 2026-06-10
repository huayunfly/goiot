// Session manager
// @date 2023.04.26

const crypto = require('node:crypto');

class Session {
    constructor(id, lastAccessTime, username) {
        this.id = id;
        this.lastAccessTime = lastAccessTime;
        this.username = username;
        this.data = new Map();
    }
}

class SessionManager {
    constructor(timeoutSeconds = 600) {
        this.sessions_ = new Map();           // id -> Session
        this.timeoutMs_ = timeoutSeconds * 1000;
    }

    createSession(username) {
        if (!username) throw new Error('Username is required');
        const s_id = crypto.randomUUID().replaceAll("-", "");
        this.sessions_.set(s_id, new Session(s_id, Date.now(), username));
        return s_id;
    }

    touchSession(s_id) {
        const session = this.sessions_.get(s_id);
        if (!session) return null;

        // Outdated session, remove it and return null to indicate invalid session.
        if (Date.now() - session.lastAccessTime > this.timeoutMs_) {
            this.sessions_.delete(s_id);
            return null;
        }

        session.lastAccessTime = Date.now();
        return session.username;
    }

    setSessionData(s_id, key, value) {
        const session = this.sessions_.get(s_id);
        if (!session) return false;
        session.data.set(key, value);
        return true;
    }

    getSessionData(s_id, key) {
        const session = this.sessions_.get(s_id);
        return session ? session.data.get(key) : undefined;
    }

    cleanupAllSessions() {
        const now = Date.now();
        const expiredIds = [];
        
        // Firstly collect expired session IDs to avoid the map iteration error.
        for (const [s_id, session] of this.sessions_) {
            if (now - session.lastAccessTime > this.timeoutMs_) {
                expiredIds.push(s_id);
            }
        }
        
        for (const id of expiredIds) {
            this.sessions_.delete(id);
        }
    }

    // Display active session count.
    get activeCount() {
        return this.sessions_.size;
    }
}

module.exports.SessionManager = SessionManager;
