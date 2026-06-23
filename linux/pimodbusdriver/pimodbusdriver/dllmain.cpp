// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <memory>
#include "../../pidataservice/pidataservice/driver_base.h"
#include "ModbusRtuDriver.h"


void __attribute__((constructor)) MyLoad(void);
void __attribute__((destructor)) MyUnload(void);


std::unique_ptr<goiot::DriverBase> GetObj(void) {
    return std::make_unique<goiot::ModbusRtuDriver>();
}

std::string GetName(void) {
    return "modbus_rtu";
}

// Called when the library is loaded and before dlopen() returns
void MyLoad(void)
{
    // Add initialization code…
}

// Called when the library is unloaded and before dlclose()
// returns
void MyUnload(void)
{
    // Add clean-up code…
}

