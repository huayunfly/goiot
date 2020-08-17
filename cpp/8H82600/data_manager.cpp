#include <iostream>
#include "data_manager.h"

namespace goiot {

const std::wstring DataManager::CONFIG_FILE = L"drivers.json";
const std::wstring DataManager::DRIVER_DIR = L"drivers";
const std::string DataManager::HMSET_STRING_FORMAT = "HMSET %s value %s result %d time %f";
const std::string DataManager::HMSET_INTEGER_FORMAT = "HMSET %s value %d result %d time %f";
const std::string DataManager::HMSET_FLOAT_FORMAT = "HMSET %s value %f result %d time %f";
const std::string DataManager::REDIS_PING = "PING";
const std::string DataManager::REDIS_PONG = "PONG";

const std::string DataManager::NS_REFRESH = "refresh:";
const std::string DataManager::NS_POLL = "poll:";
const std::string DataManager::NS_REFRESH_TIME = "time_r:";
const std::string DataManager::NS_POLL_TIME = "time_p:";
const std::string DataManager::NS_DELIMITER = ":";

void DataManager::ConnectRedis()
{
    initRedisClient();
    redis_refresh_.reset(new RedisClient::Connection(redis_config_));
    bool connected = redis_refresh_->connect();
    if (!connected)
    {
        std::cout << "redis refresh_ connection failed" << std::endl;
    }

    redis_poll_.reset(new RedisClient::Connection(redis_config_));
    connected = redis_poll_->connect();
    if (!connected)
    {
        std::cout << "redis poll_ connection failed" << std::endl;
    }

    //        // Run command and wait for result
    //        RedisClient::Response rep = redis_refresh_->commandSync({"PING"});

    //    // Run command in async mode
    //    redis_refresh_->command({"PING"});

    //    // Run command in db #2
    //    redis_refresh_->command({"PING"}, 2);

    //    // Run async command with callback
    //    redis_refresh_->command({"PING"}, nullptr, [](RedisClient::Response r, QString s) {
    //        QVariant val = r.value(); // get value from response
    //        std::cout << val.String << std::endl;
    //        // do stuff
    //    });
}

bool DataManager::ConnectedRedis(std::shared_ptr<RedisClient::Connection> redis_connection)
{
    if (!redis_connection)
    {
        return false;
    }

    RedisClient::Response reply = redis_connection->commandSync({REDIS_PING.c_str()});
    if (reply.isValid() && reply.type() == RedisClient::Response::Status &&
            reply.value().toString().compare(REDIS_PONG.c_str()) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/// Start working threads
void DataManager::Start()
{
    ConnectRedis();
    keep_refresh_ = true;
    threads_.emplace_back(std::thread(&DataManager::RefreshDispatch, this));
}

/// Stop working threads
void DataManager::Stop()
{
    keep_refresh_ = false;
    for (auto& entry : threads_)
    {
        entry.join();
    }
}

/// Get Refresh data from redis and put into the in-queue.
void DataManager::RefreshDispatch()
{
    const double DEADBAND = 1e-3;
    const double TIMESPAN = 1000000.0; // in second

    while(keep_refresh_)
    {
        if (ConnectedRedis(redis_refresh_))
        {
            const std::string HMGET = "hmget"; // hmget key field [field]
            const std::string ZRANGE_BY_SCORES = "zrangebyscore"; // zrangebyscore key min max
            double now = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
            double last = now - TIMESPAN;
            std::ostringstream oss_now;
            std::ostringstream oss_last;
            oss_now << now;
            oss_last << last;

            auto reply = redis_refresh_->commandSync(
            {ZRANGE_BY_SCORES.c_str(), NS_REFRESH_TIME.c_str(), oss_last.str().c_str(), oss_now.str().c_str()});

            std::vector<std::string> member_vec;
            if (reply.isValid() && reply.type() == RedisClient::Response::Array)
            {
                auto list = reply.value().toStringList();

                for (int i = 0; i < list.size(); i++)
                {
                    if (true)
                    {
                        member_vec.emplace_back(list.at(i).toUtf8().constData());
                    }
                }
            }

            std::vector<DataInfo> data_info_vec;
            // pipeline
            RedisClient::Command cmd;
            for (auto& member : member_vec)
            {
                cmd.addToPipeline({HMGET.c_str(), member.c_str(), "value", "result"});
            }
            reply = redis_refresh_->commandSync(cmd);
            if (reply.isValid() && reply.type() == RedisClient::Response::Array)
            {
                auto list = reply.value().toList();
                assert(list.size() == (int)member_vec.size());
                if (list.size() != (int)member_vec.size())
                {
                    throw std::runtime_error("pipeline reply is not match with request.");
                }
                // Get replies from pipeline
                for (int i = 0; i < list.size(); i++)
                {
                    auto inner_list = list.at(i).toStringList();
                    if (inner_list.size() != 2)
                    {
                        assert(false);
                        continue;
                    }
                    std::string value = inner_list.at(0).toUtf8().constData();
                    int result = inner_list.at(1).toInt();

                    // Remove namespace, for example: poll:mfcpfc.4.sv -> mfcpfc.4.sv
                    std::size_t namespace_pos = member_vec.at(i).find_first_of(":");
                    namespace_pos = namespace_pos < 0 ? 0 : namespace_pos + 1;
                    std::string data_info_id = member_vec.at(i).substr(namespace_pos, member_vec.at(i).size() - namespace_pos);
                    auto data_info = data_info_cache_.FindEntry(data_info_id);
                    if (data_info.id.empty())
                    {
                        assert(false);
                        continue; // Throw exception
                    }
                    if (data_info.read_write_priviledge == ReadWritePrivilege::READ_ONLY)
                    {
                        assert(false);
                        continue; // Throw exception
                    }

                    double timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
                    if (data_info.data_type == DataType::DF)
                    {
                        // Deadband
                        double new_value = std::atof(value.c_str());
                        if (std::abs(data_info.float_value - new_value) < DEADBAND && data_info.result == 0)
                        {
                            continue;
                        }
                        else // data_info.result != 0
                        {
                            data_info.float_value = new_value; // update poll
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);

                        }
                        data_info_vec.emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::REFRESH, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       0/* integer */, new_value, ""/* string */, timestamp
                                                       );
                    }
                    else if (data_info.data_type == DataType::STR)
                    {
                        std::string& new_value = value;
                        if (data_info.char_value.compare(new_value) == 0 && data_info.result == 0)
                        {
                            continue;
                        }
                        else
                        {
                            data_info.char_value = new_value; // update poll
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
                        }
                        data_info_vec.emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       0/* integer */, 0.0/* float */, new_value, timestamp
                                                       );
                    }
                    else if (data_info.data_type == DataType::BT)
                    {
                        uint8_t new_value = std::atoi(value.c_str()) > 0 ? 1 : 0;
                        if (data_info.byte_value == new_value && data_info.result == 0)
                        {
                            continue;
                        }
                        else
                        {
                            data_info.byte_value = new_value; // updata poll
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
                        }
                        data_info_vec.emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, new_value,
                                                       0/* integer */, 0.0/* float */, "", timestamp
                                                       );
                    }
                    else
                    {
                        int new_value = std::atoi(value.c_str());
                        if (data_info.int_value == new_value && data_info.result == 0)
                        {
                            continue;
                        }
                        else
                        {
                            data_info.int_value = new_value;
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
                        }
                        data_info_vec.emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       new_value, 0.0/* float */, "", timestamp
                                                       );
                    }
                }
            }

            // Dispatch writing data to in-queue
            if (data_info_vec.size() > 0)
            {
                request_queue_->Put(std::make_shared<std::vector<DataInfo>>(data_info_vec));
            }
        }
        else
        {
            ConnectRedis();
        }
    }
}

/// Check response queue and write changed data to redis poll zone.
void DataManager::ResponseDispatch()
{

}

/// Check request queue and update UI using QT message.
void DataManager::RequestDispatch()
{

}

}
