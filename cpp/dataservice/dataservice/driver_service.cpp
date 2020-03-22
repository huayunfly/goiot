/**
 * driver_service.cpp: A driver manager service, dealing with load configuration,
 * load drivers, read/write/refresh data.
 *
 * @author Yun Hua
 * @version 0.1 2020.03.22
 */

#include <fstream>
#include <iostream>
#include <system_error>
#include "json/json.h"
#include "driver_service.h"


const std::wstring goiot::DriverMgrService::CONFIG_FILE = L"drivers.json";
const std::wstring goiot::DriverMgrService::DRIVER_DIR = L"drivers";

namespace goiot {

    int DriverMgrService::LoadJsonConfig()
    {
        if (module_path_.empty())
        {
            std::cout << "DriverMgrService::LoadJsonConfig() module_path is empty" << std::endl;
            return ENOENT; // no_such_file_or_directory
        }

        std::fstream fstream(module_path_ + CONFIG_FILE);
        if (!fstream)
        {
            std::cout << "DriverMgrService::LoadJsonConfig() no_such_file_or_directory" << std::endl;
            return ENOENT;  // add error log here
        }
        std::stringstream sstream;
        sstream << fstream.rdbuf();
        std::string rawjson = sstream.str();
        
        // Json parse
        const auto rawjson_length = static_cast<int>(rawjson.length());
        Json::String err;
        Json::Value root;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(rawjson.c_str()/* start */, rawjson.c_str() + rawjson_length/* end */,
            &root, &err))
        {
            std::cout << "json parse error" << std::endl;
            return ECANCELED;
        }
        const std::string name = root["name"].asString();
        std::cout << name << std::endl;
        return 0;
    }

    int DriverMgrService::GetPlugins()
    {
        return 0;
    }
}
