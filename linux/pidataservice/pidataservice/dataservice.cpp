// dataservice.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "driver_base.h"
#include "driver_service.h"

#include <unistd.h>

#include <vector>
#include <memory>
#include <stdexcept>
#include <exception>
#include <iostream>

// Get the module file name: /home/pi/dataservice/dataservice
std::string GetModuleFileName()
{
    const int MAX_SIZE = 260;
    char current_absolute_path[MAX_SIZE];
    int cnt = readlink("/proc/self/exe", current_absolute_path, MAX_SIZE);
    if (cnt < 0 || cnt >= MAX_SIZE)
    {
        std::cerr << "***GetModuleFileName() Error***" << std::endl;
        return std::string{};
    }
    return std::string{ current_absolute_path };
}

int main()
{
    //获取当前目录绝对路径，即去掉程序名
    std::string exe_path = GetModuleFileName();
    std::size_t pos = exe_path.find_last_of("/");
    std::string module_path = exe_path.substr(0, pos + 1);

    // Create a driver manager
    std::unique_ptr<goiot::DriverMgrService> driver_manager(new goiot::DriverMgrService(module_path));
    driver_manager->LoadJsonConfig();
    driver_manager->GetPlugins();
    driver_manager->Start();
    std::cout << "driver manager is running...";
    char c;
    while (std::cin >> c)
    {
        if (c == 'q')
        {
            driver_manager->Stop();
            return 0;
        }
    }
}
