// @purpose A data manager read/write redis.
// @author huayunfly at 126.com
// @date 2020.08.15

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include "qredisclient/include/redisclient.h"
#include "driver_base.h"

class MainWindow;

namespace goiot {
class DataManager
{
public:
    DataManager(const std::string& module_path) : module_path_(module_path) ,
        redis_config_("127.0.0.1"), redis_refresh_(), redis_poll_(),
        request_queue_(std::make_shared<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>>(10)),
        response_queue_(std::make_shared<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>>(10)),
        keep_refresh_(false)
    {
        redis_config_.setTimeouts(1500, 1500);
        initRedisClient();
    }

    DataManager(const std::string& module_path, const RedisClient::ConnectionConfig& redis_config) :
        module_path_(module_path), redis_config_(redis_config), redis_refresh_(), redis_poll_(),
        response_queue_(std::make_shared<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>>(10)),
        keep_refresh_(false)
    {
        initRedisClient();
    }
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    /// Load json file, the same with data service.
    int LoadJsonConfig();

    /// Start working threads
    void Start();

    /// Stop working threads
    void Stop();

    /// <summary>
    /// Read data from cache by id and return value directly.
    /// </summary>
    /// <param name="data_id_vec">data id vector</param>
    /// <returns>A DataInfo vector. If the data id is found, it returns a valid DataInfo.
    /// Otherwise, it returns an empty DataInfo.</returns>
    std::vector<DataInfo>&& ReadDataCache(const std::vector<std::string>& data_id_vec);

    /// Write data to response queue.
    /// <param name="data_info_vec">data info vector</param>
    /// <returns> 0 if the data is put into the queue. Otherwise -1 </returns>
    int WriteDataAsync(const std::vector<DataInfo>& data_info_vec);

    /// Register UI refresh function.
    /// <param name="func">function object</param>
    void RegisterRefreshFunc(std::function<void(std::shared_ptr<std::vector<DataInfo>>)> func);

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
    const static std::string CONFIG_FILE;
    const static std::string REDIS_PING;
    const static std::string REDIS_PONG;
    const static std::string NS_REFRESH;
    const static std::string NS_POLL;
    const static std::string NS_REFRESH_TIME;
    const static std::string NS_POLL_TIME;
    const static std::string NS_DELIMITER;


private:
    DataEntryCache<DataInfo> data_info_cache_;

    std::string module_path_; // Not include the suffix "/"
    RedisClient::ConnectionConfig redis_config_;
    std::shared_ptr<RedisClient::Connection> redis_refresh_;
    std::shared_ptr<RedisClient::Connection> redis_poll_;
    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> request_queue_;
    std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue_;
    std::vector<std::thread> threads_;
    bool keep_refresh_;

    std::function<void(std::shared_ptr<std::vector<DataInfo>>)> refresh_func_;
};
}

#endif // DATAMANAGER_H
