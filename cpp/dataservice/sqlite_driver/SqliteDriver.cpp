#include "pch.h"
#include <iostream>
#include <cassert>
#include "SqliteDriver.h"

namespace goiot
{
	RESULT_DSAPI SqliteDriver::InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info)
	{
        if (response_queue == nullptr)
        {
            throw std::invalid_argument("Parameter response_queue is null.");
        }
        ConnectionInfo connection_details;
        std::map<std::string, DataInfo> data_map;
        int parse_code = ParseConfig(config, connection_details, data_map);
        if (parse_code != 0)
        {
            std::cout << "SqliteDriver::InitDriver() parse config failed." << std::endl;
        }
        // Set data_info in Driver manager
        for (auto& pair : data_map)
        {
            pair.second.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
            set_data_info(pair.second);
        }

        _driver_worker.reset(new SqliteDriverWorker(connection_details, std::move(data_map), response_queue, _id));
        int return_code = _driver_worker->OpenConnection();
        std::cout << "SqliteDriver::InitDriver() returns " << return_code << std::endl;
        _driver_worker->Start();
        _worker_ready = true;
        return 0;
	}

	RESULT_DSAPI SqliteDriver::UnitDriver()
	{
        if (_worker_ready)
        {
            _worker_ready = false;
            _driver_worker->Stop();
            _driver_worker->CloseConnection();
            _driver_worker.reset();
        }
        std::cout << "SqliteDriver::UnitDriver() done." << std::endl;
        return 0;
	}

	RESULT_DSAPI SqliteDriver::AsyncWrite(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id)
	{
        if (!_worker_ready) // Todo: race condition with Stop()
        {
            return ENOTCONN;
        }
        return _driver_worker->AsyncWrite(data_info_vec, 0/* reserved transaction id */);
	}

	RESULT_DSAPI SqliteDriver::AsyncRead(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id, std::function<void(std::vector<DataInfo>&&, uint64_t)> read_callback)
	{
        throw std::runtime_error("Not implemented.");
	}

	int SqliteDriver::ParseConfig(const std::string& config,
		ConnectionInfo& connection_info, std::map<std::string, DataInfo>& data_map)
	{
        // Json parse
        const auto rawjson_length = static_cast<int>(config.length());
        Json::String err;
        Json::Value root;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(config.c_str()/* start */, config.c_str() + rawjson_length/* end */,
            &root, &err))
        {
            std::cout << "SqliteDriver::ParseConfig() json parse error" << std::endl;
            return ECANCELED;
        }
        // Driver ID
        _id = root["id"].asString();

        // Get port as the DB pathname
        std::size_t start_pos = 0;
        const std::string port = root["port"].asString();
        if (port.empty())
        {
            assert(false);
            std::cout << "SqliteDriver::ParseConfig() port empty." << std::endl;
            return EINVAL;
        }
        connection_info.port = port;

        // nodes
        assert(root["nodes"].isArray());
        for (auto node : root["nodes"])
        {
            // Address
            int address = 0;
            std::istringstream iss_address(node["address"].asString());
            iss_address >> address;
            // Nodes
            assert(node["data"].isArray());
            for (auto data_node : node["data"])
            {
                DataInfo data_info;
                data_info.id = root["id"].asString() + "." + node["address"].asString() + "." + data_node["id"].asString();
                data_info.name = data_node["name"].asString();
                data_info.address = address;
                data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
                if (data_node.isMember("ratio") && data_node["ratio"].isDouble())
                {
                    float ratio = data_node["ratio"].asFloat();
                    if (std::abs(ratio) > 1e-6) // Ignore zero, keep ratio to default 1.0
                    {
                        data_info.ratio = ratio;
                    }
                }
                std::string channel = data_node["register"].asString();
                assert(channel.length() > 3);
                if (channel.length() <= 3)
                {
                    std::cout << "SqliteDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                    continue;
                }
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
                    std::cout << "SqliteDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                    continue;
                }
                std::istringstream iss_data_zone(channel.substr(start_pos, 1));
                int int_zone = 0;
                iss_data_zone >> int_zone;
                switch (int_zone)
                {
                case 6:
                    data_info.data_zone = DataZone::DB;
                    break;
                default:
                    assert(false);
                    data_info.data_zone = DataZone::DB;
                    break;
                }
                start_pos++;
                if (data_info.data_zone == DataZone::DB)
                {
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
                    else if (data_type.find("BB") == 0)
                    {
                        data_info.data_type = DataType::BB;
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
                        std::cout << "SqliteDriver::ParseConfig() data.register " << data_type << " is invalid." << std::endl;
                        continue;
                    }
                }
                std::istringstream iss_register_address(channel.substr(start_pos, 4));
                iss_register_address >> data_info.register_address;
                data_map.insert(std::pair<std::string, DataInfo>(data_info.id, data_info));
            }
        }
        return 0;
	}
}
