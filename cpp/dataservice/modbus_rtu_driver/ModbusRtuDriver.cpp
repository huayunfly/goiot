// @author: Yun Hua (huayunfly@126.com)
// @version: 0.1 2020.06.27

#include "pch.h"
#include "ModbusRtuDriver.h"

namespace goiot
{
    // Interface methods
    RESULT_DSAPI ModbusRtuDriver::InitDriver(const std::string& config, 
        std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue,
        std::function<void(const DataInfo&)> set_data_info)
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
            std::cout << "ModbusRtuDriver::InitDriver() parse config failed." << std::endl;
        }
        // Set data_info in Driver manager
        for (auto& pair : data_map)
        {
            pair.second.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
            set_data_info(pair.second);
        }
        driver_worker_.reset(new DriverWorker(connection_details, std::move(data_map), response_queue));
        int return_code = driver_worker_->OpenConnection();
        std::cout << "ModbusRtuDriver::InitDriver() returns " << return_code << std::endl;
        driver_worker_->Start();
        worker_ready_ = true;
        return 0;
    }

    RESULT_DSAPI ModbusRtuDriver::UnitDriver()
    {
        if (worker_ready_)
        {
            worker_ready_ = false;
            driver_worker_->Stop();
            driver_worker_.reset();
        }
        std::cout << "ModbusRtuDriver::UnitDriver() done." << std::endl;
        return 0;
    }

    RESULT_DSAPI ModbusRtuDriver::AsyncWrite(const std::vector<DataInfo>& data_info_vec)
    {
        if (!worker_ready_) // Todo: race condition with Stop()
        {
            return ENOTCONN;
        }
        driver_worker_->AsyncWrite(data_info_vec, 0/* reserved transaction id */);
        return 0;
    }

    RESULT_DSAPI ModbusRtuDriver::AsyncRead(const std::vector<std::string> id_vec,
        std::function<void(std::vector<DataInfo>&&)> read_callback)
    {
        throw std::runtime_error("Not implemented.");
    }

    // Private methods
    int ModbusRtuDriver::ParseConfig(const std::string& config,
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
            std::cout << "ModbusRtuDriver::ParseConfig() json parse error" << std::endl;
            return ECANCELED;
        }
        // Driver ID
        id_ = root["id"].asString();

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
            std::cout << "ModbusRtuDriver::ParseConfig() connection information parsing failed." << std::endl;
            return EINVAL;
        }
        std::istringstream iss(port.substr(start_pos, split_pos - start_pos));
        iss >> connection_info.stop_bit;
        connection_info.response_to_msec = root["response_to_msec"].asInt();

        // nodes
        assert(root["nodes"].isArray());
        for (auto node : root["nodes"])
        {
            // Address
            int address = 0;
            std::istringstream iss_address(node["address"].asString());
            iss_address >> address;
            // Float decode
            std::istringstream iss_float_decode(node["fdec"].asString());
            int float_decode_int = 0;
            iss_float_decode >> float_decode_int;
            FloatDecode float_decode;
            switch (float_decode_int)
            {
            case 0:
                float_decode = FloatDecode::ABCD;
                break;
            case 1:
                float_decode = FloatDecode::DCBA;
                break;
            case 2:
                float_decode = FloatDecode::BADC;
                break;
            case 3:
                float_decode = FloatDecode::CDAB;
                break;
            default:
                throw std::invalid_argument("Invalid float decode.");
            }
            // Nodes
            assert(node["data"].isArray());
            for (auto data_node : node["data"])
            {
                DataInfo data_info;
                data_info.id = root["id"].asString() + "." + node["address"].asString() + "." + data_node["id"].asString();
                data_info.name = data_node["name"].asString();
                data_info.address = address;
                data_info.float_decode = float_decode;
                data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
                float ratio = data_node["ratio"].asFloat();
                if (std::abs(ratio) > 1e-6) // Ignore zero, keep ratio to default 1.0
                {
                    data_info.ratio = ratio;
                }
                std::string channel = data_node["register"].asString();
                assert(channel.length() > 6);
                if (channel.length() <= 6)
                {
                    std::cout << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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
                    std::cout << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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
                    std::cout << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
                    break;
                }
                start_pos++;
                if (data_info.data_zone == DataZone::INPUT_REGISTER || data_info.data_zone == DataZone::OUTPUT_REGISTER)
                {
                    std::string data_type = channel.substr(start_pos, 3);
                    if (data_type.find_first_of("DF") == 0)
                    {
                        data_info.data_type = DataType::DF;
                        start_pos += 2;
                    }
                    else if (data_type.find_first_of("WUB") == 0)
                    { 
                        data_info.data_type = DataType::WUB;
                        start_pos += 3;
                    }
                    else if (data_type.find_first_of("WB") == 0)
                    {
                        data_info.data_type = DataType::WB;
                        start_pos += 2;
                    }
                    else if (data_type.find_first_of("DUB") == 0)
                    {
                        data_info.data_type = DataType::DUB;
                        start_pos += 3;
                    }
                    else if (data_type.find_first_of("DB") == 0)
                    {
                        data_info.data_type = DataType::DB;
                        start_pos += 2;
                    }
                    else if (data_type.find_first_of("BT") == 0)
                    {
                        data_info.data_type = DataType::BT;
                        start_pos += 2;
                    }
                    else if (data_type.find_first_of("STR") == 0)
                    {
                        data_info.data_type = DataType::STR;
                        start_pos += 3;
                    }
                    else
                    {
                        assert(false);
                        std::cout << "ModbusRtuDriver::ParseConfig() data.register " << channel << " is invalid." << std::endl;
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

