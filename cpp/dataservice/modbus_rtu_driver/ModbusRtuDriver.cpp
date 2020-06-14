#include "pch.h"
#include "ModbusRtuDriver.h"

namespace goiot
{
    // Interface methods
    RESULT_DSAPI ModbusRtuDriver::InitDriver(const std::string& config)
    {
        ConnectionInfo connection_details;
        std::map<std::string, DataInfo> data_map;
        int parse_code = ParseConfig(config, connection_details, data_map);
        if (parse_code != 0)
        {
            std::clog << "ModbusRtuDriver::InitDriver() parse config failed." << std::endl;
        }
        driver_worker_.reset(new DriverWorker(connection_details, std::move(data_map)));
        int return_code = driver_worker_->OpenConnection();
        std::cout << "ModbusRtuDriver::InitDriver() returns " << return_code << std::endl;
        return 0;
    }

    RESULT_DSAPI ModbusRtuDriver::UnitDriver()
    {
        std::cout << "ModbusRtuDriver::UnitDriver() done." << std::endl;
        return 0;
    }

    // Private methods
    int ModbusRtuDriver::ParseConfig(const std::string& config,
        ConnectionInfo connection_info, std::map<std::string, DataInfo> data_map)
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
            std::cout << "ModbusRtuDriver::ParseConfig() json parse error" << std::endl;
            return ECANCELED;
        }
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
            std::clog << "ModbusRtuDriver::ParseConfig() connection information parsing failed." << std::endl;
            return EINVAL;
        }
        std::istringstream iss(port.substr(start_pos, split_pos - start_pos));
        iss >> connection_info.stop_bit;
        connection_info.response_to_msec = root["response_to_msec"].asInt();

        // nodes
        assert(root["nodes"].isArray());
        for (auto node : root["nodes"])
        {
            assert(node["data"].isArray());
            for (auto data_node : node["data"])
            {
                DataInfo data_info;
                data_info.id = root["id"].asString() + "." + node["address"].asString() + "." + data_node["id"].asString();
                data_info.name = data_node["name"].asString();
                std::istringstream iss_address(node["address"].asString());
                iss_address >> data_info.address;
                data_info.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                std::string channel = data_node["register"].asString();
                assert(channel.length() > 6);
                if (channel.length() <= 6)
                {
                    std::clog << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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
                    std::clog << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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
                default:
                    assert(false);
                    std::clog << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                    break;
                }
                start_pos++;
                if (data_info.data_zone == DataZone::INPUT_REGISTER || data_info.data_zone == DataZone::OUTPUT_REGISTER)
                {
                    std::string data_type = channel.substr(start_pos, 3);
                    if (data_type.compare("DF") == 0)
                    {
                        data_info.data_type = DataType::DF;
                        start_pos += 2;
                    }
                    else if (data_type.compare("WUB") == 0)
                    {
                        data_info.data_type = DataType::WUB;
                        start_pos += 3;
                    }
                    else if (data_type.compare("WB") == 0)
                    {
                        data_info.data_type = DataType::WB;
                        start_pos += 2;
                    }
                    else if (data_type.compare("DUB") == 0)
                    {
                        data_info.data_type = DataType::DUB;
                        start_pos += 3;
                    }
                    else if (data_type.compare("DB") == 0)
                    {
                        data_info.data_type = DataType::DB;
                        start_pos += 2;
                    }
                    else
                    {
                        assert(false);
                        std::clog << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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

