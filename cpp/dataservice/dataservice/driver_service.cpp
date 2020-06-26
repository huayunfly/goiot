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
#include <cassert>
#include <Windows.h>

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
		// driver descriptions
		assert(root["drivers"].isArray());
		driver_descriptions_.clear();
		for (auto driver : root["drivers"])
		{
			auto description = std::make_tuple(driver["type"].asString(), driver["port"].asString(), driver.toStyledString());
			if (std::find_if(driver_descriptions_.begin(), driver_descriptions_.end(),
				[&driver](std::shared_ptr<std::tuple<std::string, std::string, std::string>> pos) {
					return std::get<0>(*pos) == driver["type"].asString() && std::get<1>(*pos) == driver["port"].asString(); }) == driver_descriptions_.end())
			{
				driver_descriptions_.push_back(
					std::make_shared<std::tuple<std::string, std::string, std::string>>(description));
			}

		}

		return 0;
	}

	int DriverMgrService::GetPlugins()
	{
		std::vector<HINSTANCE> modules;
		drivers_ = GetPlugins(modules, module_path_, driver_descriptions_);
		return 0;
	}

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
	std::vector<std::unique_ptr<goiot::DriverBase>> DriverMgrService::GetPlugins(std::vector<HINSTANCE>& modules,
		const std::wstring& module_path,
		const std::vector<std::shared_ptr<std::tuple<std::string/*type*/, std::string/*port*/, std::string/*content*/>>>& driver_descriptions) {
		// A temporary structure to return.
		std::vector<std::unique_ptr<goiot::DriverBase>> driver_objs;
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
			HINSTANCE module = LoadLibrary((module_path + L"drivers\\" + std::wstring(fileData.cFileName)).c_str());

			if (!module) {
				// Couldn't load the library, cleaning module list and quitting.
				for (HINSTANCE hInst : modules)
				{
					FreeLibrary(hInst);
				}
				throw std::runtime_error("Library " + std::string(/*fileData.cFileName*/"") + " wasn't loaded successfully!");
			}

			// Get the function and the class exported by the DLL.
			// If you aren't using the MinGW compiler, you may need to adjust
			// this to cope with name mangling (I haven't gone into this here,
			// look it up if you want).
			ObjProc objFunc = (ObjProc)GetProcAddress(module, "?GetObj@@YA?AV?$unique_ptr@VDriverBase@goiot@@U?$default_delete@VDriverBase@goiot@@@std@@@std@@XZ");
			NameProc nameFunc = (NameProc)GetProcAddress(module, "?GetName@@YA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ");


			if (!objFunc || !nameFunc)
			{
				throw std::runtime_error("Invalid Plugin DLL: both 'getObj' and 'getName' must be defined.");
			}

			// push the objects and modules into our vectors
			std::string driver_name = nameFunc();
			for (auto description : driver_descriptions)
			{
				if (!std::get<0>(*description).compare(driver_name))
				{
					auto obj = objFunc();
					obj->InitDriver(std::get<2>(*description), response_queue_);
					driver_objs.push_back(std::move(obj));
					std::clog << driver_name << "|" << std::get<1>(*description) << " loaded!\n";
				}
			}
			modules.push_back(module);
		} while (FindNextFile(fileHandle, &fileData));

		std::clog << std::endl;

		// Close the file when we are done
		FindClose(fileHandle);
		return driver_objs;
	}

	void DriverMgrService::PutResponseData(std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (data_info_vec == nullptr)
		{
			throw std::invalid_argument("Parameter data_info_vec is null.");
		}
	}

	void DriverMgrService::Start()
	{
		threads_.emplace_back(std::thread(&DriverMgrService::Response_Dispatch, this));
	}

	void DriverMgrService::Stop()
	{
		response_queue_->Close();
		for (auto& entry : threads_)
		{
			entry.join();
		}
	}

	void DriverMgrService::Response_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			response_queue_->Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			std::cout << "Response from devices..." << std::endl;
		}
	}
}
