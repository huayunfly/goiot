// Session manager
// @date 2023.04.26


class Session
{
    s_id;
    last_access_time;
    constructor(id, lasttime)
    {
        this.s_id = id;
        this.last_access_time = lasttime;
    }
}


class SessionMagager
{
    constructor()
    {
        this.sessions_ = new Map();
        this.session_data_ = new Map();
        this.session_timeout_ = 600; // in second
    }

    get time_now()
    {
        return new Date().getTime() / 1000.0;
    }

    create_session()
    {
        const s_id = this.create_guid();
        this.sessions_.set(s_id, new Session(s_id, this.time_now));
        return s_id;
    }

    create_guid()
    {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
          var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
          return v.toString(16);
        });
    }

    cleanup_session(s_id)
    {
        this.session_data_.delete(s_id);
        this.sessions_.delete(s_id);
    }

    cleanup_all_sessions()
    {
        for (let s of this.sessions_)
        {
            if (s[1].last_access_time + this.session_timeout_ < this.time_now)
            {
                this.cleanup_session(s[0]);
            }
        }
    }

    touch_session(s_id)
    {
        let s = this.sessions_.get(s_id);
        if (s === undefined)
        {
            return false;
        }
        s.last_access_time = this.time_now;
        return true;
    }

    set_session_data(s_id, key, value)
    {
        const s_d = this.session_data_.get(s_id);
        if (s_d === undefined)
        {
            const data = new Map();
            data.set(key, value);
            this.session_data_.set(s_id, data);
        }
        else
        {
            s_d.set(key, value); // new or update key-value
        }
    }

    // Get the session data by ID and key, return undefined if the ID does not exist.
    get_session_data(s_id, key)
    {
        let s_d = this.session_data_.get(s_id);
        return (s_d === undefined) ? undefined : s_d.get(key);
    }
}

module.exports.SessionMagager = SessionMagager;