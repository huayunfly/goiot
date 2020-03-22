// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "../dataservice/driver_base.h"

#include <iostream>

namespace goiot
{
    class DemoDriver : public DriverBase {
    public:
        RESULT_DSAPI GetDescription(std::string& description)
        {
            description = "DemoDriver2020";
            return 0;
        }

        RESULT_DSAPI InitDriver(const std::string& config)
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
    return "DemoDriver";
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

