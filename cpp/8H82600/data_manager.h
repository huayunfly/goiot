// @purpose A data manager read/write redis.
// @author huayunfly at 126.com
// @date 2020.08.15

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <memory>
#include "qredisclient/include/asyncfuture.h"
#include "qredisclient/include/redisclient.h"

namespace goiot {
class DataManager
{
public:
    DataManager() : redis_config_("127.0.0.1"), redis_refresh_()
    {
        redis_config_.setTimeouts(2000, 2000);
    }

    DataManager(const RedisClient::ConnectionConfig& redis_config) :
        redis_config_(redis_config), redis_refresh_()
    {
    }
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    void Connect();
    void Disconnect();

private:
    RedisClient::ConnectionConfig redis_config_;
    std::shared_ptr<RedisClient::Connection> redis_refresh_;
};
}

#endif // DATAMANAGER_H
