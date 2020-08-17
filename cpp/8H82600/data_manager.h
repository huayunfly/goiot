// @purpose A data manager read/write redis.
// @author huayunfly at 126.com
// @date 2020.08.15

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include "qredisclient/include/redisclient.h"
#include "driver_base.h"

namespace goiot {
class DataManager
{
public:
    DataManager() : redis_config_("127.0.0.1"), redis_refresh_(), redis_poll_(),
        keep_refresh_(false)
    {
        redis_config_.setTimeouts(1500, 1500);
    }

    DataManager(const RedisClient::ConnectionConfig& redis_config) :
        redis_config_(redis_config), redis_refresh_(), redis_poll_(), keep_refresh_(false)
    {
    }
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    /// Start working threads
    void Start();

    /// Stop working threads
    void Stop();

private:
    /// Connect redis with refresh and poll connections.
    void ConnectRedis();

    /// Using ping to test redis connection status.
    bool ConnectedRedis(std::shared_ptr<RedisClient::Connection> redis_connection);

    /// Get Refresh data from redis and put into the in-queue.
    void RefreshDispatch();

    /// Check response queue and write changed data to redis poll zone.
    void ResponseDispatch();

    /// Check request queue and update UI using QT message.
    void RequestDispatch();

private:
    const static std::wstring CONFIG_FILE;
    const static std::wstring DRIVER_DIR;
    const static std::string HMSET_STRING_FORMAT;
    const static std::string HMSET_INTEGER_FORMAT;
    const static std::string HMSET_FLOAT_FORMAT;
    const static std::string REDIS_PING;
    const static std::string REDIS_PONG;
    const static std::string NS_REFRESH;
    const static std::string NS_POLL;
    const static std::string NS_REFRESH_TIME;
    const static std::string NS_POLL_TIME;
    const static std::string NS_DELIMITER;


private:
    DataEntryCache<DataInfo> data_info_cache_;

    RedisClient::ConnectionConfig redis_config_;
    std::shared_ptr<RedisClient::Connection> redis_refresh_;
    std::shared_ptr<RedisClient::Connection> redis_poll_;
    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue_;
    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> request_queue_;
    std::vector<std::thread> threads_;
    bool keep_refresh_;
};
}

#endif // DATAMANAGER_H
