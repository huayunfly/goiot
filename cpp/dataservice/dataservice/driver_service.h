/**
 * driver_service.h: A driver manager service, dealing with load configuration,
 * load drivers, read/write/refresh data.
 *
 * @author: Yun Hua (huayunfly@126.com)
 * @version 0.1 2020.03.22
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "driver_base.h"
#include "ThreadSafeQueue.h"
#include "hiredis.h"

namespace goiot {
	class DriverMgrService
	{
	public:
		DriverMgrService(const std::wstring& module_path) : module_path_(module_path),
			drivers_(), driver_descriptions_(), 
			response_queue_(std::make_shared<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>>(1000)),
			threads_(), redis_push_(), redis_push_ready_(false), redis_poll_ready_(false), keep_poll_(false)
		{

		}

		DriverMgrService(const DriverMgrService& service) = delete;
		DriverMgrService& operator=(const DriverMgrService& service) = delete;

		~DriverMgrService()
		{

		}


	public:
		// Load json configuration from the default position
		// @return <error_code>
		int LoadJsonConfig();
		// Get driver plugins according to the json config.
		// @return <error_code>
		int GetPlugins();
		/// <summary>
		/// Start collecting data from devices and dispatching data.
		/// Start reading and writing redis.
		/// </summary>
		void Start();
		/// <summary>
		/// Stop dispatching data.
		/// </summary>
		void Stop();

		/// <summary>
		/// Put data in a response queue.
		/// </summary>
		/// <param name="data_info_vec">A DataInfo vector</param>
		void PutResponseData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);

	private:
		/// <summary>
		/// A callback for driver add DataInfo objects.
		/// </summary>
		/// <param name="data_info">A DataInfo object.</param>
		static void AddDataInfo(const DataInfo& data_info);

	private:
		/// <summary>
		/// Dispatch worker deals with the response_queue request, which may trasnfer data to the DataService.
		/// </summary>
		void ResponseDispatch();

		/// <summary>
		/// Poll redis using a seperate context. Dispatch writing data, considering the deadband and timestamp.
		/// Deadband, if the integer or float data change is in the deadband, no request will be sent to device.
		/// Timestamp, if the data change is in the deadband but timestamp elapsed, new request will be sent to device.
		/// </summary>
		void PollDispatch();

		// Get the plugins internally
		std::vector<std::unique_ptr<goiot::DriverBase>> GetPlugins(std::vector<HINSTANCE>& modules,
			const std::wstring& module_path,
			const std::vector<std::shared_ptr<std::tuple<std::string/*type*/, std::string/*port*/, std::string/*content*/>>>& driver_descriptions);

		/// <summary>
		/// Add the redis poll set by judging DataInfo is not existed and DataInfo.datatype = W or RW
		/// Call once only.
		/// </summary>
		void AddRedisPollSet();


	private:
		const static std::wstring CONFIG_FILE;
		const static std::wstring DRIVER_DIR;
		const static std::string HSET_STRING_FORMAT;
		const static std::string HSET_INTEGER_FORMAT;
		const static std::string HSET_FLOAT_FORMAT;
		const static std::string HKEY_REFRESH;
		const static std::string HKEY_POLL;

	private:
		static DataEntryCache<DataInfo> data_info_cache_;

		std::wstring module_path_; // Not include the suffix "/"
		std::vector<std::shared_ptr<std::tuple<std::string/*type*/, std::string/*port*/, std::string/*content*/>>> driver_descriptions_;
		std::vector<std::unique_ptr<DriverBase>> drivers_;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue_;
		std::vector<std::thread> threads_;
		std::shared_ptr<redisContext> redis_push_;
		std::shared_ptr<redisContext> redis_poll_;
		bool redis_push_ready_;
		bool redis_poll_ready_;
		bool keep_poll_;
	};
}


