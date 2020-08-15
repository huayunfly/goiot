#include <iostream>
#include "data_manager.h"

namespace goiot {

void DataManager::Connect()
{
    initRedisClient();
    redis_refresh_.reset(new RedisClient::Connection(redis_config_));
    bool connected = redis_refresh_->connect();
    if (!connected)
    {
        std::cout << "redis connection failed" << std::endl;
    }
    // Run command and wait for result
    RedisClient::Response rep = redis_refresh_->commandSync({"PING"});

    // Run command in async mode
    redis_refresh_->command({"PING"});

    // Run command in db #2
    redis_refresh_->command({"PING"}, 2);

    // Run async command with callback
    redis_refresh_->command({"PING"}, nullptr, [](RedisClient::Response r, QString s) {
      QVariant val = r.value(); // get value from response
      std::cout << val.String << std::endl;
      // do stuff
    });
}

void DataManager::Disconnect()
{

}

}
