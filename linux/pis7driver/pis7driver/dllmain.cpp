// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <memory>
#include "../../pidataservice/pidataservice/driver_base.h"
#include "S7Driver.h"


void __attribute__((constructor)) MyLoad(void);
void __attribute__((destructor)) MyUnload(void);


std::unique_ptr<goiot::DriverBase> GetObj(void) {
    return std::make_unique<goiot::S7Driver>();
}

std::string GetName(void) {
    return "s7";
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

