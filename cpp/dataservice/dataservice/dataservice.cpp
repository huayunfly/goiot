// dataservice.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <Windows.h>

#if defined(_MSC_VER)
// Used to install a report hook that prevent dialog on assertion and error.
#include <crtdbg.h>
#endif // if defined(_MSC_VER)

#include "driver_base.h"
#include "driver_service.h"

// Load the objects from the plugin folder.
//
// Takes as a parameter a reference to a list of modules,
// which will be emptied and then refilled with handles to
// the modules loaded. These should be freed with the
// FreeLibrary() after use.
//
// Returns a list of Base*, contained in a smart pointer
// to ease memory deallocation and help prevent memory
// leaks.
std::vector<std::unique_ptr<goiot::DriverBase>> GetPlugins(std::vector<HINSTANCE>& modules, 
    const std::wstring& module_path) {
    // A temporary structure to return.
    std::vector<std::unique_ptr<goiot::DriverBase>> ret;
    // empty the modules list passed
    modules.clear();
    // Get full path of the file  
    WIN32_FIND_DATA fileData;
    HANDLE fileHandle = FindFirstFile((module_path + L"drivers\\*.dll").c_str(), &fileData);

    if (fileHandle == (void*)ERROR_INVALID_HANDLE ||
        fileHandle == (void*)ERROR_FILE_NOT_FOUND) {
        // If we couldn't find any plugins, quit gracefully,
        // returning an empty structure.
        return std::vector<std::unique_ptr<goiot::DriverBase>>();
    }

    // Loop over every plugin in the folder and store in our
    // temporary return list
    do {
        // Define the function types for what we are retrieving
        typedef std::unique_ptr<goiot::DriverBase>(__cdecl* ObjProc)(void);
        typedef std::string(__cdecl* NameProc)(void);

        // Load the library
        HINSTANCE mod = LoadLibrary((module_path + L"drivers\\" + std::wstring(fileData.cFileName)).c_str());

        if (!mod) {
            // Couldn't load the library, cleaning module list and quitting.
            for (HINSTANCE hInst : modules)
                FreeLibrary(hInst);
            throw std::runtime_error("Library " + std::string(/*fileData.cFileName*/"") + " wasn't loaded successfully!");
        }

        // Get the function and the class exported by the DLL.
        // If you aren't using the MinGW compiler, you may need to adjust
        // this to cope with name mangling (I haven't gone into this here,
        // look it up if you want).
        ObjProc objFunc = (ObjProc)GetProcAddress(mod, "?GetObj@@YA?AV?$unique_ptr@VDriverBase@goiot@@U?$default_delete@VDriverBase@goiot@@@std@@@std@@XZ");
        NameProc nameFunc = (NameProc)GetProcAddress(mod, "?GetName@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");


        if (!objFunc || !nameFunc)
        {
            throw std::runtime_error("Invalid Plugin DLL: both 'getObj' and 'getName' must be defined.");
        }

        // push the objects and modules into our vectors
        ret.push_back(objFunc());
        modules.push_back(mod);

        std::clog << nameFunc() << " loaded!\n";
    } while (FindNextFile(fileHandle, &fileData));

    std::clog << std::endl;

    // Close the file when we are done
    FindClose(fileHandle);
    return ret;
}

int TestObjs(const std::wstring& module_path)
{
    // Our list of modules. We need this to properly free the module
// after the program has finished.
    std::vector<HINSTANCE> modules;

    // Enter a block, to prevent the Base objects being
    // deallocated after the modules are freed (which will
    // cause your program to crash)
    {
        // The list of objectects that we will retrieve from the DLL's.
        std::vector<std::unique_ptr<goiot::DriverBase>> objs;

        // Load the plugins using our function
        try
        {
            objs = GetPlugins(modules, module_path);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            return 1;
        }

        // Call the 'print' function for out classes.
        // This is normally where you would implement
        // the code for using the plugins.
        for (auto& x : objs)
        {
            std::string description;
            std::cout << x->GetDescription(description) << std::endl;
            x->InitDriver(R"({"addr":"1"})", nullptr/* DriverMgrService only */);
            x->UnitDriver();
        }
    }

    // Program finishing, time to clean up
    for (HINSTANCE hInst : modules)
    {
        FreeLibrary(hInst);
    }
}

int main()
{
    // Module path
    wchar_t exeFullPath[MAX_PATH]; // Full path   
    GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    std::wstring exePath(exeFullPath);
    std::size_t pos = exePath.find_last_of(L"\\");
    std::wstring module_path = exePath.substr(0, pos + 1);
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
