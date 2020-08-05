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

	DataEntryCache<DataInfo> DriverMgrService::data_info_cache_;
	const std::wstring DriverMgrService::CONFIG_FILE = L"drivers.json";
	const std::wstring DriverMgrService::DRIVER_DIR = L"drivers";
	const std::string DriverMgrService::HMSET_STRING_FORMAT = "HMSET %s value %s result %d time %f";
	const std::string DriverMgrService::HMSET_INTEGER_FORMAT = "HMSET %s value %d result %d time %f";
	const std::string DriverMgrService::HMSET_FLOAT_FORMAT = "HMSET %s value %f result %d time %f";
	const std::string DriverMgrService::HKEY_REFRESH = "goiot_r";
	const std::string DriverMgrService::HKEY_POLL = "goiot_p";

	const std::string DriverMgrService::NS_REFRESH = "refresh:";
	const std::string DriverMgrService::NS_POLL = "poll:";
	const std::string DriverMgrService::NS_REFRESH_TIME = "time_r:";
	const std::string DriverMgrService::NS_POLL_TIME = "time_p:";
	const std::string DriverMgrService::NS_DELIMITER = ":";

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
					obj->InitDriver(std::get<2>(*description), response_queue_, &DriverMgrService::AddDataInfo);
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
		ConnectRedis();
		// Add(Initialize) Writable and ReadWritable data into poll zone.
		if (redis_poll_ready_)
		{
			AddRedisSet(redis_poll_, NS_POLL_TIME, NS_POLL, true);
		}
		if (redis_refresh_ready_)
		{
			AddRedisSet(redis_refresh_, NS_REFRESH_TIME, NS_REFRESH, false);
		}
		// Start Resonse dispatch thread.
		threads_.emplace_back(std::thread(&DriverMgrService::ResponseDispatch, this));
		// Poll thread.
		keep_poll_ = true;
		threads_.emplace_back(std::thread(&DriverMgrService::PollDispatch, this));
	}

	void DriverMgrService::ConnectRedis()
	{
		// For redis push
		struct timeval timeout = { 1, 500000 }; // 1.5 seconds
		redis_refresh_.reset(redisConnectWithTimeout("127.0.0.1", 6379, timeout),
			[](redisContext* p) { redisFree(p); });
		if (redis_refresh_ != nullptr && redis_refresh_->err)
		{
			redis_refresh_ready_ = false;
			std::cout << "Redis refresh connection error: " << redis_refresh_->errstr << std::endl;
			// handle error
		}
		else if (redis_refresh_ == nullptr)
		{
			redis_refresh_ready_ = false;
			std::cout << "Redis refresh connection error: can't allocate redis context" << std::endl;
		}
		else
		{
			redis_refresh_ready_ = true;
			std::cout << "Redis refresh connection OK." << std::endl;
		}
		// Poll
		redis_poll_.reset(redisConnectWithTimeout("127.0.0.1", 6379, timeout),
			[](redisContext* p) { redisFree(p); });
		if (redis_poll_ != nullptr && redis_poll_->err)
		{
			redis_poll_ready_ = false;
			std::cerr << "Redis poll connection error: " << redis_refresh_->errstr << std::endl;
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
	}

	void DriverMgrService::Stop()
	{
		// Stop the polling thread.
		keep_poll_ = false;
		// Uninitialize drivers
		for (auto& driver : drivers_)
		{
			driver->UnitDriver();
		}
		// Stop the response dispatch thread.
		response_queue_->Close();

		for (auto& entry : threads_)
		{
			entry.join();
		}
		redis_refresh_ready_ = false;
		redis_poll_ready_ = false;
		redis_refresh_.reset();
		redis_poll_.reset();
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
			if (redis_refresh_ready_)
			{
				for (auto& data_info : *data_info_vec)
				{
					if (data_info.result != 0)
					{
						continue; // Todo: handle error code.
					}
					
					std::string refresh_id = NS_REFRESH + data_info.id;
					if (data_info.data_type == DataType::STR)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_refresh_.get(), HMSET_STRING_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), refresh_id.c_str(), data_info.char_value.c_str(), data_info.result, data_info.timestamp)
							), [](redisReply* p) { freeReplyObject(p); });
					}
					else if (data_info.data_type == DataType::DF)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_refresh_.get(), HMSET_FLOAT_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), refresh_id.c_str(), data_info.float_value, data_info.result, data_info.timestamp)
							), [](redisReply* p) { freeReplyObject(p); });
					}
					else if (data_info.data_type == DataType::DB || data_info.data_type == DataType::DUB ||
						data_info.data_type == DataType::WB || data_info.data_type == DataType::WUB ||
						data_info.data_type == DataType::BT)
					{
						std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
							redisCommand(redis_refresh_.get(), HMSET_INTEGER_FORMAT.c_str(),
								HKEY_REFRESH.c_str(), refresh_id.c_str(), data_info.int_value, data_info.result, data_info.timestamp)
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
		const double DEADBAND = 1e-3;
		const double TIMESPAN = 10.0; // in second
		while (keep_poll_)
		{
			if (redis_poll_ready_)
			{
				const std::string HMGET = "hmget %s %s %s"; // hmget key field [field]
				const std::string ZRANGE_BY_SCORES = "zrangebyscore %s %f %f"; // zrangebyscore key min max
				double now = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
				double last = now - TIMESPAN;
				std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
					redisCommand(redis_poll_.get(), ZRANGE_BY_SCORES.c_str(), HKEY_POLL.c_str(), last, now)
					), [](redisReply* p) { freeReplyObject(p); });
				std::vector<std::string> member_vec;
				if (reply->type == REDIS_REPLY_ARRAY)
				{
					//std::vector<DataInfo> data_info_vec;
					for (int i = 0; i < reply->elements; i++)
					{
						if (reply->element[i]->type == REDIS_REPLY_STRING)
						{
							member_vec.emplace_back(reply->element[i]->str);
						}
					}
				}

				std::unordered_map<std::string, std::vector<DataInfo>> data_info_group;
				for (auto& member : member_vec)
				{
					// Remove namespace, for example: poll:mfcpfc.4.sv -> mfcpfc.4.sv
					std::size_t namespace_pos = member.find_first_of(":");
					namespace_pos = namespace_pos < 0 ? 0 : namespace_pos + 1;
					std::string data_info_id = member.substr(namespace_pos, member.size() - namespace_pos);
					auto data_info = data_info_cache_.FindEntry(data_info_id);
					if (data_info.id.empty())
					{
						assert(false);
						continue; // Throw exception
					}
					if (data_info.read_write_priviledge == ReadWritePrivilege::READ_ONLY)
					{
						assert(false);
						continue; // Throw exception
					}
					// Get Value
					std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
						redisCommand(redis_poll_.get(), HMGET.c_str(), member, "value", "result")
						), [](redisReply* p) { freeReplyObject(p); });
					std::string value;
					int result;
					if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
					{
						value = reply->element[0]->str;
						result = atoi(reply->element[1]->str);
					}
					else
					{
						assert(false);
						continue;
					}
					// Option: check result code
					if (result != 0)
					{
						assert(false);
						continue;
					}
					// Parse driver type id seperated by ".", for example: mfcpfc.4.sv -> mfcpfc
					std::size_t seperator_pos = data_info.id.find_first_of(".");
					std::size_t len = seperator_pos < 0 ? data_info.id.size() : seperator_pos;
					std::string driver_id = data_info.id.substr(0, len);
					// About unordered_map.emplace()
					// If the insertion takes place (because no other element existed with the same key), the function returns a pair object, whose first component is an iterator to the inserted element, and whose second component is true.
					// Otherwise, the pair object returned has as first component an iterator pointing to the element in the container with the same key, and false as its second component.
					auto group_pos = data_info_group.emplace(driver_id, std::vector<DataInfo>()).first;
					double timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
					if (data_info.data_type == DataType::DF)
					{
						// Deadband
						double new_value = std::atof(value.c_str());
						if (std::abs(data_info.float_value - new_value) < DEADBAND && data_info.result == 0)
						{
							continue;
						}
						else // data_info.result != 0
						{
							data_info.float_value = new_value; // update poll
							data_info_cache_.UpateOrAddEntry(data_info.id, data_info);

						}
						group_pos->second.emplace_back(data_info.id,
							data_info.name, data_info.address, data_info.register_address,
							data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
							data_info.data_zone, data_info.float_decode, 0/* byte */,
							0/* integer */, new_value, ""/* string */, timestamp
						);
					}
					else if (data_info.data_type == DataType::STR)
					{
						std::string& new_value = value;
						if (data_info.char_value.compare(new_value) == 0 && data_info.result == 0)
						{
							continue;
						}
						else
						{
							data_info.char_value = new_value; // update poll
							data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
						}
						group_pos->second.emplace_back(data_info.id,
							data_info.name, data_info.address, data_info.register_address,
							data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
							data_info.data_zone, data_info.float_decode, 0/* byte */,
							0/* integer */, 0.0/* float */, new_value, timestamp
						);
					}
					else if (data_info.data_type == DataType::BT)
					{
						byte new_value = std::atoi(value.c_str());
						if (data_info.byte_value == new_value && data_info.result == 0)
						{
							continue;
						}
						else
						{
							data_info.byte_value = new_value; // updata poll
							data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
						}
						group_pos->second.emplace_back(data_info.id,
							data_info.name, data_info.address, data_info.register_address,
							data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
							data_info.data_zone, data_info.float_decode, new_value,
							0/* integer */, 0.0/* float */, "", timestamp
						);
					}
					else
					{
						int new_value = std::atoi(value.c_str());
						if (data_info.int_value == new_value && data_info.result == 0)
						{
							continue;
						}
						else
						{
							data_info.int_value = new_value;
							data_info_cache_.UpateOrAddEntry(data_info.id, data_info);
						}
						group_pos->second.emplace_back(data_info.id,
							data_info.name, data_info.address, data_info.register_address,
							data_info.read_write_priviledge, DataFlowType::ASYNC_WRITE, data_info.data_type,
							data_info.data_zone, data_info.float_decode, 0/* byte */,
							new_value, 0.0/* float */, "", timestamp
						);
					}
                }
                // Dispatch writing data to devices
                for (auto& element : data_info_group)
                {
                    for (auto& driver : drivers_)
                    {
                        std::string id;
                        driver->GetID(id);
                        if (id.compare(element.first) == 0 && element.second.size() > 0)
                        {
                            driver->AsyncWrite(element.second, 0); // Todo: transaction id
                            break;
                        }
                    }
                }
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}

	// Static method
	void DriverMgrService::AddDataInfo(const DataInfo& data_info)
	{
		if (data_info.id.empty())
		{
			throw std::invalid_argument("Parameter data_info ID is empty.");
		}
		data_info_cache_.AddEntry(data_info.id, data_info);
	}

	void DriverMgrService::AddRedisSet(std::shared_ptr<redisContext> redis_context,
		const std::string& time_namespace, const std::string& key_namespace, bool poll_set)
	{
		//const std::string HGETALL = "HGETALL %s";
		const std::string ZRANGE_ALL = "zrange %s 0 -1 withscores"; // zrange key start end [withscores]
		const std::string ZADD = "zadd %s %f %s"; // zadd key score member [score member...]
		const std::string HMSET_STRING = "hmset %s id %s name %s value %s rw %d result %d time %f"; // hmset key field value [field value...]
		const std::string HMSET_FLOAT = "hmset %s id %s name %s value %f rw %d result %d time %f"; // hmset key field value [field value...]
		const std::string HMSET_INTEGER = "hmset %s id %s name %s value %d rw %d result %d time %f"; // hmset key field value [field value...]

		std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
			redisCommand(redis_context.get()/* redisContext */, ZRANGE_ALL.c_str()/* format */, time_namespace.c_str())
			), [](redisReply* p) { freeReplyObject(p); });
		//1) "poll:mfcpfc.4.pv"
		//2) "1596273994.957"
		//3) "poll:mfcpfc.4.sv"
		//4) "1596273994.957"
		std::unordered_set<std::string> existed_ids;
		if (reply->type == REDIS_REPLY_ARRAY)
		{
			int hset_number = reply->elements / 2;  // field:value pair number
			std::unordered_map<std::string, std::vector<DataInfo>> data_info_group;
			for (int i = 0; i < hset_number; i++)
			{
				if (reply->element[i * 2]->type == REDIS_REPLY_STRING)
				{
					// Trim namespace prefix
					std::string key(reply->element[i * 2]->str);
					std::size_t pos = key.find(key_namespace);
					if (pos >= 0)
					{
						key = key.substr(pos + key_namespace.size(), key.size() - key_namespace.size());
					}
					existed_ids.insert(key);
				}
			}
		}
		auto data_info_ids = data_info_cache_.GetEntryKeys();
		for (auto& id : existed_ids)
		{
			auto it = data_info_ids.find(id);
			if (it != data_info_ids.end())
			{
				data_info_ids.erase(id);
			}
		}
		for (auto& id : data_info_ids)
		{
			auto data_info = data_info_cache_.FindEntry(id);
			if (!data_info.id.empty() &&
				(!poll_set || (poll_set && data_info.read_write_priviledge != ReadWritePrivilege::READ_ONLY)
					)
				)
			{
				if (data_info.data_type == DataType::STR)
				{
					std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
						redisCommand(redis_context.get(), HMSET_STRING.c_str(),
						(key_namespace + data_info.id).c_str(),
							data_info.id.c_str(),
							data_info.name.c_str(),
							data_info.char_value.c_str(),
							data_info.read_write_priviledge,
							data_info.result,
							data_info.timestamp)
						), [](redisReply* p) { freeReplyObject(p); });
					CheckRedisReply(data_info.id, "AddRedisPollSet", reply.get());
				}
				else if (data_info.data_type == DataType::DF)
				{
					std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
						redisCommand(redis_context.get(), HMSET_FLOAT.c_str(),
						(key_namespace + data_info.id).c_str(),
							data_info.id.c_str(),
							data_info.name.c_str(),
							data_info.float_value,
							data_info.read_write_priviledge,
							data_info.result,
							data_info.timestamp)
						), [](redisReply* p) { freeReplyObject(p); });
					CheckRedisReply(data_info.id, "AddRedisPollSet", reply.get());
				}
				else if (data_info.data_type == DataType::DB || data_info.data_type == DataType::DUB ||
					data_info.data_type == DataType::WB || data_info.data_type == DataType::WUB)
				{
					std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
						redisCommand(redis_context.get(), HMSET_INTEGER.c_str(),
						(key_namespace + data_info.id).c_str(),
							data_info.id.c_str(),
							data_info.name.c_str(),
							data_info.int_value,
							data_info.read_write_priviledge,
							data_info.result,
							data_info.timestamp)
						), [](redisReply* p) { freeReplyObject(p); });
					CheckRedisReply(data_info.id, "AddRedisPollSet", reply.get());
				}
				else if (data_info.data_type == DataType::BT)
				{
					std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
						redisCommand(redis_context.get(), HMSET_INTEGER.c_str(),
						(key_namespace + data_info.id).c_str(),
							data_info.id.c_str(),
							data_info.name.c_str(),
							data_info.byte_value,
							data_info.read_write_priviledge,
							data_info.result,
							data_info.timestamp)
						), [](redisReply* p) { freeReplyObject(p); });
					CheckRedisReply(data_info.id, "AddRedisPollSet", reply.get());
				}
				else
				{
					throw std::invalid_argument("Unsupported data type.");
				}
				// add to ZSET time_p:
				std::unique_ptr<redisReply, void(*)(redisReply*)> reply(static_cast<redisReply*>(
					redisCommand(redis_context.get(), ZADD.c_str(),
						time_namespace.c_str(),
						data_info.timestamp,
						(key_namespace + data_info.id).c_str()
					)
					), [](redisReply* p) { freeReplyObject(p); });
				CheckRedisReply(data_info.id, "AddRedisPollSet", reply.get());
			}
		}
	}

	void DriverMgrService::CheckRedisReply(const std::string& id, const std::string& operation, const redisReply* reply) const
	{
		assert(reply->type != REDIS_REPLY_ERROR);
#ifdef _DEBUG
		if (reply->type == REDIS_REPLY_ERROR && reply->str)
		{
			std::cout << "redis reply error: [" << id << "]" << " [" + operation + "] " + reply->str << std::endl;
		}
#endif // _DEBUG
	}
}
