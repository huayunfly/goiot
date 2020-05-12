// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "../dataservice/driver_base.h"

#include <iostream>

namespace goiot
{
    class ModbusRtuDriver : public DriverBase {
    public:
        RESULT_DSAPI GetDescription(std::string& description)
        {
            description = "Modbus_Rtu";
            return 0;
        }

        RESULT_DSAPI InitDriver(const std::string& config)
        {
            std::cout << "ModbusRtuDriver::InitDriver() " + config << std::endl;
            return 0;
        }

        RESULT_DSAPI UnitDriver()
        {
            std::cout << "ModbusRtuDriver::UnitDriver()" << std::endl;
            return 0;
        }
    };
}

std::unique_ptr<goiot::DriverBase> GetObj(void) {
    return std::make_unique<goiot::ModbusRtuDriver>();
}

std::string GetName(void) {
    return "ModbusRtuDriver";
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

