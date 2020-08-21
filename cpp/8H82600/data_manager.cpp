#include <iostream>
#include <fstream>
#include <QJsonDocument>
#include <QJsonArray>
#include "data_manager.h"

namespace goiot {

const std::string DataManager::CONFIG_FILE = "\\drivers.json";

const std::string DataManager::REDIS_PING = "PING";
const std::string DataManager::REDIS_PONG = "PONG";

const std::string DataManager::NS_REFRESH = "refresh:";
const std::string DataManager::NS_POLL = "poll:";
const std::string DataManager::NS_REFRESH_TIME = "time_r:";
const std::string DataManager::NS_POLL_TIME = "time_p:";
const std::string DataManager::NS_DELIMITER = ":";

void DataManager::ConnectRedis()
{
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

/// Load json file, the same with data service.
int DataManager::LoadJsonConfig()
{
    if (module_path_.empty())
    {
        std::cout << "DataManager::LoadJsonConfig() module_path is empty" << std::endl;
        return ENOENT; // no_such_file_or_directory
    }

    std::fstream fstream(module_path_ + CONFIG_FILE);
    if (!fstream)
    {
        std::cout << "DataManager::LoadJsonConfig() no_such_file_or_directory" << std::endl;
        return ENOENT;  // add error log here
    }
    std::stringstream sstream;
    sstream << fstream.rdbuf();

    // Json parse
    QString json = sstream.str().c_str();
    QJsonParseError error;
    auto json_doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !json_doc.isObject())
    {
        std::cout << "Loading json is invalid." << std::endl;
        return -1;
    }
    auto root = json_doc.object();
    if (root.contains("drivers") && root["drivers"].isArray())
    {
        auto drivers = root["drivers"].toArray();
        for (const auto& driver : drivers)
        {
            if (!driver.isObject())
            {
                assert(false);
                continue;
            }
            auto driver_obj = driver.toObject();
            if (!driver_obj.contains("id"))
            {
                assert(false);
                continue;
            }
            std::string driver_id = driver_obj["id"].toString().toStdString();
            if (!driver_obj.contains("nodes") || !driver_obj["nodes"].isArray())
            {
                assert(false);
                continue;
            }
            auto nodes = driver_obj["nodes"].toArray();
            for (const auto& node : nodes)
            {
                if (!node.isObject())
                {
                    assert(false);
                    continue;
                }
                auto node_obj = node.toObject();
                if (!node_obj.contains("address") || !node_obj.contains("data") || !node_obj["data"].isArray())
                {
                    assert(false);
                    continue;
                }
                int address = node_obj["address"].toString().toInt();
                auto data_vec = node_obj["data"].toArray();
                for (const auto& data : data_vec)
                {
                    if (!data.isObject())
                    {
                        assert(false);
                        continue;
                    }
                    auto data_obj = data.toObject();
                    if (!data_obj.contains("id") || !data_obj.contains("name") || !data_obj.contains("register"))
                    {
                        assert(false);
                        continue;
                    }
                    std::ostringstream oss;
                    oss << driver_id << "." << address << "." << data_obj["id"].toString().toStdString();
                    DataInfo data_info;
                    data_info.id = oss.str();
                    data_info.name = data_obj["name"].toString().toStdString();
                    data_info.address = address;
                    data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
                    if (data_obj.contains("ratio"))
                    {
                        data_info.ratio = data_obj["ratio"].toDouble();
                    }
                    // data type
                    auto channel = data_obj["register"].toString().toStdString();
                    assert(channel.length() > 6);
                    if (channel.length() <= 6)
                    {
                        std::cout << "DataManager::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                        continue;
                    }
                    int start_pos = 0;
                    if (0 == channel.find("RW"))
                    {
                        data_info.read_write_priviledge = ReadWritePrivilege::READ_WRITE;
                        start_pos = 2;
                    }
                    else if (0 == channel.find("R"))
                    {
                        data_info.read_write_priviledge = ReadWritePrivilege::READ_ONLY;
                        start_pos = 1;

                    }
                    else if (0 == channel.find("W"))
                    {
                        data_info.read_write_priviledge = ReadWritePrivilege::WRITE_ONLY;
                        start_pos = 1;
                    }
                    else
                    {
                        assert(false);
                        std::cout << "DataManager::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                        continue;
                    }
                    std::istringstream iss_data_zone(channel.substr(start_pos, 1));
                    int int_zone = 0;
                    iss_data_zone >> int_zone;
                    switch (int_zone)
                    {
                    case 0:
                        data_info.data_zone = DataZone::OUTPUT_RELAY;
                        break;
                    case 1:
                        data_info.data_zone = DataZone::INPUT_RELAY;
                        break;
                    case 3:
                        data_info.data_zone = DataZone::INPUT_REGISTER;
                        break;
                    case 4:
                        data_info.data_zone = DataZone::OUTPUT_REGISTER;
                        break;
                    case 5:
                        data_info.data_zone = DataZone::PLC_DB;
                        break;
                    default:
                        assert(false);
                        std::cout << "DataManager::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                        break;
                    }

                    start_pos++;
                    std::string data_type = channel.substr(start_pos, 3);
                    if (data_type.find("DF") == 0)
                    {
                        data_info.data_type = DataType::DF;
                        start_pos += 2;
                    }
                    else if (data_type.find("WUB") == 0)
                    {
                        data_info.data_type = DataType::WUB;
                        start_pos += 3;
                    }
                    else if (data_type.find("WB") == 0)
                    {
                        data_info.data_type = DataType::WB;
                        start_pos += 2;
                    }
                    else if (data_type.find("DUB") == 0)
                    {
                        data_info.data_type = DataType::DUB;
                        start_pos += 3;
                    }
                    else if (data_type.find("DB") == 0)
                    {
                        data_info.data_type = DataType::DB;
                        start_pos += 2;
                    }
                    else if (data_type.find("BT") == 0)
                    {
                        data_info.data_type = DataType::BT;
                        start_pos += 2;
                    }
                    else if (data_type.find("STR") == 0)
                    {
                        data_info.data_type = DataType::STR;
                        start_pos += 3;
                    }
                    else
                    {
                        assert(false);
                        std::cout << "DataManager::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                        continue;
                    }
                    data_info_cache_.AddEntry(data_info.id, data_info);
                }
            }
        }
    }
    return 0;
}

/// Start working threads
void DataManager::Start()
{
    ConnectRedis();
    keep_refresh_ = true;
    threads_.emplace_back(std::thread(&DataManager::RefreshDispatch, this));
    threads_.emplace_back(std::thread(&DataManager::RequestDispatch, this));
    threads_.emplace_back(std::thread(&DataManager::ResponseDispatch, this));
}

/// Stop working threads
void DataManager::Stop()
{
    keep_refresh_ = false;
    request_queue_->Close();
    response_queue_->Close();

    for (auto& entry : threads_)
    {
        entry.join();
    }
}

/// Read data from cache by id and return value directly.
void DataManager::ReadDataCache(std::vector<DataInfo>& data_info_vec)
{
    for (auto& data_info : data_info_vec)
    {
        auto entry = data_info_cache_.FindEntry(data_info.id);
        if (!entry.id.empty())
        {
            data_info = entry;
        }
        else
        {
            assert(false);
        }
    }
}

/// Write data to response queue
int DataManager::WriteDataAsync(const std::vector<DataInfo>& data_info_vec)
{
    if (data_info_vec.size() > 0)
    {
        try
        {
            response_queue_->PutNoWait(std::make_shared<std::vector<DataInfo>>(data_info_vec));
            return 0;
        }
        catch (const QFull&)
        {

        }
        return EWOULDBLOCK;
    }
    return ENODATA;
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
                        member_vec.emplace_back(list.at(i).toStdString());
                    }
                }
            }

            auto data_info_vec = std::make_shared<std::vector<DataInfo>>();
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
                if (list.size() != (int)member_vec.size())
                {
                    assert(false);
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

                    // Remove namespace, for example: poll:mfcpfc.4.sv -> mfcpfc.4.sv
                    std::size_t namespace_pos = member_vec.at(i).find_first_of(":");
                    namespace_pos = (namespace_pos == std::string::npos) ? 0 : namespace_pos + 1;
                    std::string data_info_id = member_vec.at(i).substr(namespace_pos, member_vec.at(i).size() - namespace_pos);
                    auto data_info = data_info_cache_.FindEntry(data_info_id);
                    if (data_info.id.empty())
                    {
                        assert(false);
                        continue; // Throw exception
                    }
                    std::string value = inner_list.at(0).toStdString();
                    data_info.result = inner_list.at(1).toInt(); // Notify whether the data is healthy.
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
                            data_info.float_value = new_value; // update refresh
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);

                        }
                        data_info_vec->emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::REFRESH, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       0/* integer */, new_value, ""/* string */, timestamp, data_info.result, data_info.ratio
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
                            data_info.char_value = new_value; // update refresh
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
                        }
                        data_info_vec->emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::REFRESH, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       0/* integer */, 0.0/* float */, new_value, timestamp, data_info.result, data_info.ratio
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
                            data_info.byte_value = new_value; // updata refresh
                            data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
                        }
                        data_info_vec->emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::REFRESH, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, new_value,
                                                       0/* integer */, 0.0/* float */, "", timestamp, data_info.result, data_info.ratio
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
                        data_info_vec->emplace_back(data_info.id,
                                                       data_info.name, data_info.address, data_info.register_address,
                                                       data_info.read_write_priviledge, DataFlowType::REFRESH, data_info.data_type,
                                                       data_info.data_zone, data_info.float_decode, 0/* byte */,
                                                       new_value, 0.0/* float */, "", timestamp, data_info.result, data_info.ratio
                                                       );
                    }
                }
            }

            // Dispatch writing data to in-queue
            if (data_info_vec->size() > 0)
            {
                request_queue_->Put(data_info_vec);
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
    while (true)
    {
        std::shared_ptr<std::vector<DataInfo>> data_info_vec;
        response_queue_->Get(data_info_vec);
        if (data_info_vec == nullptr) // Improve for a robust SENTINEL
        {
            break; // Exit
        }

        //double now = std::chrono::duration_cast<std::chrono::milliseconds>(
        //	std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
        if (ConnectedRedis(redis_poll_))
        {
            const std::string HMSET = "hmset"; // hmset key field value [field value]
            RedisClient::Command cmd;
            for (auto& data_info : *data_info_vec)
            {
                if (data_info.data_flow_type != DataFlowType::ASYNC_WRITE)
                {
                    continue;
                }
                // Write to redis poll zone.
                std::string poll_id = NS_POLL + data_info.id; // id
                std::ostringstream oss_result; // result
                oss_result << data_info.result;
                std::ostringstream oss_time; // timestamp
                oss_time << data_info.timestamp;
                std::ostringstream oss_value; // value
                if (data_info.data_type == DataType::STR)
                {
                    oss_value << data_info.char_value;
                    cmd.addToPipeline({HMSET.c_str(), poll_id.c_str(), "value", oss_value.str().c_str(),
                               "result", oss_result.str().c_str(), "time", oss_time.str().c_str()});

                }
                else if (data_info.data_type == DataType::DF)
                {
                    oss_value << data_info.float_value;
                    cmd.addToPipeline({HMSET.c_str(), poll_id.c_str(), "value", oss_value.str().c_str(),
                               "result", oss_result.str().c_str(), "time", oss_time.str().c_str()});

                }
                else if (data_info.data_type == DataType::BT)
                {
                    oss_value << data_info.byte_value;
                    cmd.addToPipeline({HMSET.c_str(), poll_id.c_str(), "value", oss_value.str().c_str(),
                               "result", oss_result.str().c_str(), "time", oss_time.str().c_str()});
                }
                else if (data_info.data_type == DataType::DB || data_info.data_type == DataType::DUB ||
                    data_info.data_type == DataType::WB || data_info.data_type == DataType::WUB)
                {
                    oss_value << data_info.int_value;
                    cmd.addToPipeline({HMSET.c_str(), poll_id.c_str(), "value", oss_value.str().c_str(),
                               "result", oss_result.str().c_str(), "time", oss_time.str().c_str()});
                }
                else
                {
                    throw std::invalid_argument("Unsupported data type.");
                }
            }
            // Send commands and get reply.
            if (!cmd.isEmpty())
            {
                auto reply = redis_poll_->commandSync(cmd);
                if (!reply.isValid() || reply.type() != RedisClient::Response::Array)
                {
                    assert(false);
                    std::cout << "ResponseDispatch() commandSync failed." << std::endl;
                }
            }
        }
        else
        {
            ConnectRedis();
        }
        //double gap = std::chrono::duration_cast<std::chrono::milliseconds>(
        //	std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0 - now;
        //std::cout << gap << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Avoid redis performance problem
    }
}

/// Check request queue and update UI using QT message.
void DataManager::RequestDispatch()
{
    while (true)
    {
        std::shared_ptr<std::vector<DataInfo>> data_info_vec;
        request_queue_->Get(data_info_vec);
        if (data_info_vec == nullptr) // Improve for a robust SENTINEL
        {
            break; // Exit
        }
    }
}

}