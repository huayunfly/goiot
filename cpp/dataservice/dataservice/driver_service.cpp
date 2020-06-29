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

namespace goiot {

	std::unordered_map<std::string, DataInfo> goiot::DriverMgrService::data_info_hash_;

	const std::wstring goiot::DriverMgrService::CONFIG_FILE = L"drivers.json";
	const std::wstring goiot::DriverMgrService::DRIVER_DIR = L"drivers";
	const std::string goiot::DriverMgrService::HSET_STRING_FORMAT = "HSET %s %s %s";
	const std::string goiot::DriverMgrService::HSET_INTEGER_FORMAT = "HSET %s %s %d";
	const std::string goiot::DriverMgrService::HSET_FLOAT_FORMAT = "HSET %s %s %f";
	const std::string goiot::DriverMgrService::HKEY_REFRESH = "goiot_r";
	const std::string goiot::DriverMgrService::HKEY_POLL = "goiot_p";

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
					obj->InitDriver(std::get<2>(*description), response_queue_, &DriverMgrService::SetDataInfo);
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
		// For redis
		struct timeval timeout = { 1, 500000 }; // 1.5 seconds
		redis_push_.reset(redisConnectWithTimeout("127.0.0.1", 6379, timeout), 
			[](redisContext* p) { redisFree(p); });
		if (redis_push_ != nullptr && redis_push_->err) 
		{
			redis_push_ready_ = false;
			std::cerr << "Redis refresh connection error: " << redis_push_->errstr << std::endl;
			// handle error
		}
		else if (redis_push_ == nullptr)
		{
			redis_push_ready_ = false;
			std::cerr << "Redis refresh connection error: can't allocate redis context" << std::endl;
		}
		else
		{
			redis_push_ready_ = true;
			std::cout << "Redis refresh connection OK." << std::endl;
		}
		// Poll
		redis_poll_.reset(redisConnectWithTimeout("127.0.0.1", 6379, timeout),
			[](redisContext* p) { redisFree(p); });
		if (redis_poll_ != nullptr && redis_poll_->err)
		{
			redis_poll_ready_ = false;
			std::cerr << "Redis poll connection error: " << redis_push_->errstr << std::endl;
			// handle error
		}
		else if (redis_poll_ == nullptr)
		{
			redis_poll_ready_ = false;
			std::cerr << "Redis poll connection error: can't allocate redis context" << std::endl;
		}
		else
		{
			redis_poll_ready_ = true;
			std::cout << "Redis poll connection OK." << std::endl;
		}
		threads_.emplace_back(std::thread(&DriverMgrService::ResponseDispatch, this));
		threads_.emplace_back(std::thread(&DriverMgrService::PollDispatch, this));
		// Poll thread exit sign
		keep_poll_ = true;
	}

	void DriverMgrService::Stop()
	{
		for (auto& driver : drivers_)
		{
			driver->UnitDriver();
		}
		// Stop the response dispatch thread.
		response_queue_->Close();
		// Stop the polling thread.
		keep_poll_ = false;
		for (auto& entry : threads_)
		{
			entry.join();
		}
		redis_push_.reset();
		redis_push_ready_ = false;
		redis_poll_ready_ = false;
	}

	void DriverMgrService::ResponseDispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			response_queue_->Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
#ifdef _DEBUG
			//std::cout << "Response from device " << data_info_vec->at(0).id << std::endl;
#endif // _DEBUG
			if (redis_push_ready_)
			{
				for (auto& data_info : *data_info_vec)
				{
					if (data_info.data_type == DataType::STR)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_push_.get(), HSET_STRING_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), data_info.id.c_str(), data_info.char_value.c_str())
							), [](redisReply* p) { freeReplyObject(p); });
					}
					else if (data_info.data_type == DataType::DF)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_push_.get(), HSET_FLOAT_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), data_info.id.c_str(), data_info.float_value)
							), [](redisReply* p) { freeReplyObject(p); });
					}
					else if (data_info.data_type == DataType::DB || data_info.data_type == DataType::DUB ||
						data_info.data_type == DataType::WB || data_info.data_type == DataType::WUB ||
						data_info.data_type == DataType::BT)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_push_.get(), HSET_INTEGER_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), data_info.id.c_str(), data_info.int_value)
							), [](redisReply* p) { freeReplyObject(p); });
#ifdef _DEBUG
						if (reply->type == REDIS_REPLY_ERROR && reply->str)
						{
							std::cout << "HSET reply error: " << reply->str << std::endl;
						}
#endif // _DEBUG
					}
					else
					{
						throw std::invalid_argument("Unsupported data type.");
					}
				}
			}
		}
	}

	void DriverMgrService::PollDispatch()
	{
		while (keep_poll_)
		{
			if (redis_poll_ready_)
			{
				const std::string HGETALL = "HGETALL %s";
				std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
					redisCommand(redis_poll_.get(), HGETALL.c_str(), HKEY_REFRESH.c_str())
					), [](redisReply* p) { freeReplyObject(p); });
				if (reply->type == REDIS_REPLY_ARRAY)
				{
					int hset_number = reply->elements / 2;  // field:value pair number
					std::vector<DataInfo> data_info_vec;
					for (int i = 0; i < hset_number; i++)
					{
						if (reply->element[i * 2]->type == REDIS_REPLY_STRING && 
							reply->element[i * 2 + 1]->type == REDIS_REPLY_STRING)
						{
							auto pos = data_info_hash_.find(reply->element[i * 2]->str);
							if (pos == data_info_hash_.end())
							{
								assert(false);
								continue; // Throw exception
							}
							if (pos->second.read_write_priviledge == ReadWritePrivilege::READ_ONLY)
							{
								assert(false);
								continue; // Throw exception
							}
							if (pos->second.data_type == DataType::DF)
							{
								data_info_vec.emplace_back(reply->element[i * 2]->str,
									pos->second.name, pos->second.address, pos->second.register_address,
									pos->second.read_write_priviledge, DataFlowType::WRITE, pos->second.data_type,
									pos->second.data_zone, pos->second.float_decode,
									0/* integer */, std::atof(reply->element[i * 2 + 1]->str),
									""/* string */, std::chrono::duration_cast<std::chrono::milliseconds>(
										std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0
								);
							}
							else if (pos->second.data_type == DataType::STR)
							{
								data_info_vec.emplace_back(reply->element[i * 2]->str,
									pos->second.name, pos->second.address, pos->second.register_address,
									pos->second.read_write_priviledge, DataFlowType::WRITE, pos->second.data_type,
									pos->second.data_zone, pos->second.float_decode,
									0/* integer */, 0.0/* float */, reply->element[i * 2 + 1]->str,
									std::chrono::duration_cast<std::chrono::milliseconds>(
										std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0
								);
							}
							else
							{
								data_info_vec.emplace_back(reply->element[i * 2]->str,
									pos->second.name, pos->second.address, pos->second.register_address,
									pos->second.read_write_priviledge, DataFlowType::WRITE, pos->second.data_type,
									pos->second.data_zone, pos->second.float_decode,
									std::atoi(reply->element[i * 2 + 1]->str), 0.0/* float */, "",
									std::chrono::duration_cast<std::chrono::milliseconds>(
										std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0
								);
							}
						}
					}
					// Write data to devices
					std::unordered_map<std::string, std::vector<DataInfo>> data_info_group;

					//for (auto& obj : drivers_)
					//{
					//	std::string id;
					//	obj->GetID(id);
					//	std::find(data_info_vec.begin(), data_info_vec.end(),
					//		[&id](const DataInfo& d) { return d.id.find(id) >= 0; });
					//}
				} 
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	void DriverMgrService::SetDataInfo(const DataInfo& data_info)
	{
		assert(!data_info.id.empty());
		if (data_info.id.empty())
		{
			return;
		}
		auto pos = data_info_hash_.find(data_info.id);
		if (pos != data_info_hash_.end())
		{
			throw std::out_of_range("Data info hash contains key already.");
		}
		else
		{
			data_info_hash_[data_info.id] = data_info;
		}
	}
}
