// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <memory>
#include "../dataservice/driver_base.h"
#include "FpDriver.h"

std::unique_ptr<goiot::DriverBase> GetObj(void) {
    return std::make_unique<goiot::FpDriver>();
}

std::string GetName(void) {
    return "fpplc";
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

