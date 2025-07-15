// pidataservice.cpp: 定义应用程序的入口点。
//

#include "pidataservice.h"
#include "driver_base.h"
#include "json/json.h"
#include <unistd.h>
#include <dlfcn.h>
#include <string>
#include <memory>

// Get the module file name: /home/pi/dataservice/dataservice
std::string GetModuleFileName()
{
	const int MAX_SIZE = 260;
	char current_absolute_path[MAX_SIZE];
	//获取当前程序绝对路径
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
	Json::String err;
	Json::Value root;
	Json::CharReaderBuilder builder;
	const std::string name = root[Json::StaticString("name")].asString();
	std::cout << "Hello CMake." << std::endl;

	typedef std::unique_ptr<goiot::DriverBase>(* ObjProc)(void);
	typedef std::string(* NameProc)(void);

    //获取当前目录绝对路径，即去掉程序名
    std::string exe_path = GetModuleFileName();
    std::size_t pos = exe_path.find_last_of("/");
    std::string module_path = exe_path.substr(0, pos + 1);
	std::cout << module_path << std::endl;

	void* handle;
	void (*print_message)();
	char* error;

	std::string driver_lib_path = module_path + "drivers/";
	std::string driver_path = driver_lib_path += "libpifpdriver.so";
	// 加载共享库
	handle = dlopen(driver_lib_path.c_str(), RTLD_LAZY);
	if (!handle) 
	{
		fprintf(stderr, "%s\n", dlerror());
		return 1;
	}

	// 获取函数地址
	dlerror(); // 清除之前的错误
	void *p1 = dlsym(handle, "_Z6GetObjv");
	void *p2 = dlsym(handle, "_Z7GetNameB5cxx11v");
	if ((error = dlerror()) != NULL) 
	{
		fprintf(stderr, "%s\n", error);
		dlclose(handle);
		return 1;
	}
	ObjProc obj_func = reinterpret_cast<ObjProc>(p1);
	NameProc name_func = reinterpret_cast<NameProc>(p2);

	// 调用函数
	std::cout << name_func() << std::endl;
	//auto obj = obj_func;

	// 关闭共享库
	dlclose(handle);

	return 0;
}