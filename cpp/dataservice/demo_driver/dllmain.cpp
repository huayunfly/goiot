// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <iostream>
#include "../dataservice/driver_base.h"

namespace goiot
{
    class DemoDriver : public DriverBase {
    public:
        RESULT_DSAPI GetDescription(std::string& description)
        {
            description = "DemoDriver2020";
            return 0;
        }

        virtual RESULT_DSAPI GetID(std::string& id)
        {
            id = "demo";
            return 0;
        }

        RESULT_DSAPI InitDriver(const std::string& config, 
            std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue,
            std::function<void(const DataInfo&)> set_data_info)
        {
            std::cout << "DemoDriver::InitDriver() " + config << std::endl;
            return 0;
        }

        RESULT_DSAPI UnitDriver()
        {
            std::cout << "DemoDriver::UnitDriver()" << std::endl;
            return 0;
        }
    };
}

std::unique_ptr<goiot::DriverBase> GetObj(void) {
    return std::unique_ptr<goiot::DriverBase>(new goiot::DemoDriver);
}

std::string GetName(void) {
    return "demo";
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

