#include "pch.h"
#include <iostream>
#include <cassert>
#include "FpDriver.h"
#include "FpDriverWorker.h"

namespace goiot 
{
    RESULT_DSAPI FpDriver::InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info)
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
            std::cout << "FpDriver::InitDriver() parse config failed." << std::endl;
        }
        // Set data_info in Driver manager
        for (auto& pair : data_map)
        {
            pair.second.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
            set_data_info(pair.second);
        }

        _driver_worker.reset(new FpDriverWorker(connection_details, std::move(data_map), response_queue, _id));
        int return_code = _driver_worker->OpenConnection();
        std::cout << "FpDriver::InitDriver() returns " << return_code << std::endl;
        _driver_worker->Start();
        _worker_ready = true;
        return 0;
    }

    RESULT_DSAPI FpDriver::UnitDriver()
    {
        if (_worker_ready)
        {
            _worker_ready = false;
            _driver_worker->Stop();
            _driver_worker->CloseConnection();
            _driver_worker.reset();
        }
        std::cout << "FpDriver::UnitDriver() done." << std::endl;
        return 0;
    }

    RESULT_DSAPI FpDriver::AsyncWrite(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id)
    {
        if (!_worker_ready) // Todo: race condition with Stop()
        {
            return ENOTCONN;
        }
        return _driver_worker->AsyncWrite(data_info_vec, 0/* reserved transaction id */);
    }

    RESULT_DSAPI FpDriver::AsyncRead(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id, std::function<void(std::vector<DataInfo>&&, uint64_t)> read_callback)
    {
        throw std::runtime_error("Not implemented.");
    }

    int FpDriver::ParseConfig(const std::string& config,
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
            std::cout << "FpDriver::ParseConfig() json parse error" << std::endl;
            return ECANCELED;
        }
        // Driver ID
        _id = root["id"].asString();

        // Split port, like "COM1,9600,N,8,1"
        const std::string port = root["port"].asString();
        const std::size_t NPOS = -1;
        int count = 5;
        std::size_t start_pos = 0;
        std::size_t split_pos = NPOS;
        while ((split_pos = port.find(',', start_pos)) != NPOS && count > 0 && start_pos < port.length())
        {
            std::istringstream iss(port.substr(start_pos, split_pos - start_pos));
            switch (count)
            {
            case 5:
                iss >> connection_info.port;
                break;
            case 4:
                iss >> connection_info.baud;
                break;
            case 3:
                iss >> connection_info.parity;
                break;
            case 2:
                iss >> connection_info.data_bit;
                break;
            default:
                break;
            }
            count--;
            start_pos = split_pos + 1;
        }
        if (count > 1)
        {
            assert(false);
            std::cout << "FpDriver::ParseConfig() connection information parsing failed." << std::endl;
            return EINVAL;
        }
        std::istringstream iss(port.substr(start_pos, split_pos - start_pos));
        iss >> connection_info.stop_bit;
        if (root["response_timeout_msec"] && root["response_timeout_msec"].isInt())
        {
            connection_info.response_timeout_msec = root["response_timeout_msec"].asInt();
        }
        if (connection_info.response_timeout_msec <= 0)
        {
            connection_info.response_timeout_msec = 500;
        }
        if (root["refresh_interval_msec"] && root["refresh_interval_msec"].isInt())
        {
            connection_info.refresh_interval_msec = root["refresh_interval_msec"].asInt();
        }
        if (connection_info.refresh_interval_msec <= 0)
        {
            connection_info.refresh_interval_msec = 500;
        }

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
                data_info.offset = data_node["offset"].asFloat(); // The offset token may missing, asFloat() returns 0.0 by default.
                std::string channel = data_node["register"].asString();
                assert(channel.length() > 5); // R4R1010, R4DDF1, R4DWB2, pay attention: fp2 plc r/w float operation uses two consecutive DDF registers.
                if (channel.length() <= 5)
                {
                    std::cout << "FpDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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
                    std::cout << "FpDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                    continue;
                }
                std::istringstream iss_data_zone(channel.substr(start_pos, 1));
                int int_zone = 0;
                iss_data_zone >> int_zone;
                switch (int_zone)
                {
                case 4:
                    data_info.data_zone = DataZone::OUTPUT_REGISTER;
                    break;
                default:
                    assert(false);
                    data_info.data_zone = DataZone::OUTPUT_REGISTER;
                    break;
                }
                start_pos++;
                if (data_info.data_zone == DataZone::OUTPUT_REGISTER)
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
                        std::cout << "FpDriver::ParseConfig() data.register " << data_type << " is invalid." << std::endl;
                        continue;
                    }
                }

                if (DataType::BT == data_info.data_type) // R110E -> 110 * 16 + 14(E) = 1774
                {
                    std::string r_register = channel.substr(start_pos, 4);
                    std::istringstream iss_hex(r_register.substr(r_register.size() - 1, 1));
                    int hex_value;
                    iss_hex >> std::hex >> hex_value;
                    if (iss_hex.fail())
                    {
                        hex_value = 0;
                    }
                    std::istringstream iss_decimal(r_register.substr(0, r_register.size() - 1));
                    int decimal_value;
                    iss_decimal >> std::dec >> decimal_value;
                    data_info.register_address = decimal_value * 16 + hex_value;
                }
                else
                {
                    std::istringstream iss_register_address(channel.substr(start_pos, 5)); // D 00660
                    iss_register_address >> data_info.register_address;
                }
                data_map.insert(std::pair<std::string, DataInfo>(data_info.id, data_info));
            }
        }
        return 0;
    }
}
