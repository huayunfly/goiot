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
			response_queue_(std::make_shared<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>>(100)),
			threads_(), redis_refresh_(), redis_status_(), keep_poll_(false)
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

		/// <summary>
		/// Connect to redis with more than one connection. One for status, one for poll, and the other for refresh.
		/// Every connection context is not thread-safety. We use them without locks, so be care of access sequence.
		/// </summary>
		void ConnectRedis();

		/// <summary>
		/// Using PING to get redis connection status.
		/// </summary>
		/// <returns>true: connected, otherwise false.</returns>
		bool ConnectedRedis();

		/// <summary>
		/// Dispatch worker deals with the response_queue request, which may trasnfer data to the Other service.
		/// Use pipeline to improve redis performance.
		/// </summary>
		void ResponseDispatch();

		/// <summary>
		/// Poll redis using a seperate context. Dispatch writing data, considering the deadband and timestamp.
		/// Deadband, if the integer or float data change is in the deadband, no request will be sent to device.
		/// Timestamp, if the data change is in the deadband but timestamp elapsed, new request will be sent to device.
		/// Use pipeline to improve redis performance.
		/// </summary>
		void PollDispatch();

		// Get the plugins internally
		std::vector<std::unique_ptr<goiot::DriverBase>> GetPlugins(std::vector<HINSTANCE>& modules,
			const std::wstring& module_path,
			const std::vector<std::shared_ptr<std::tuple<std::string/*type*/, std::string/*port*/, std::string/*content*/>>>& driver_descriptions);

		/// <summary>
		/// Add the redis poll or refresh set.
		/// Poll: judging DataInfo is not existed and DataInfo.datatype = W or RW
		/// Refresh: judging DataInfo is not existed only (no regard of DataInfo.datatype = W or RW or R)
		///
		/// Redis data key example: refresh:plc.1.out1 or poll:mfcpfc.4.pv, etc.
		///
		/// <param name="redis_context">redis context</param>
		/// <param name="time_namespace">time namespace, such as time_p: or time:r</param>
		/// <param name="key_namespace">data key namespace, such as poll: or refresh:</param>
		/// <param name="poll_set">True: poll set, False: refresh set</param>
		/// </summary>
		void AddRedisSet(std::shared_ptr<redisContext> redis_context, const std::string& time_namespace,
			const std::string& key_namespace, bool poll_set);

		/// <summary>
		/// Check redis reply, log error or throw exception.
		/// </summary>
		/// <param name="id">data id</param>
		/// <param name="operation">operation</param>
		/// <param name="reply">Redis reply</param>
		void CheckRedisReply(const std::string& id, const std::string& operation, const redisReply* reply) const;

	private:
		const static std::wstring CONFIG_FILE;
		const static std::wstring DRIVER_DIR;
		const static std::string HMSET_STRING_FORMAT;
		const static std::string HMSET_INTEGER_FORMAT;
		const static std::string HMSET_FLOAT_FORMAT;
		const static std::string REDIS_PING;
		const static std::string REDIS_PONG;
		const static std::string NS_REFRESH;
		const static std::string NS_POLL;
		const static std::string NS_REFRESH_TIME;
		const static std::string NS_POLL_TIME;
		const static std::string NS_DELIMITER;

	private:
		static DataEntryCache<DataInfo> data_info_cache_;

		std::wstring module_path_; // Not include the suffix "/"
		std::vector<std::shared_ptr<std::tuple<std::string/*type*/, std::string/*port*/, std::string/*content*/>>> driver_descriptions_;
		std::vector<std::unique_ptr<DriverBase>> drivers_;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue_;
		std::vector<std::thread> threads_;
		std::shared_ptr<redisContext> redis_refresh_; // Not thread safe.
		std::shared_ptr<redisContext> redis_poll_; // Not thread safe;
		std::shared_ptr<redisContext> redis_status_; // Not thread safe, for redis PING status
		bool keep_poll_;
	};
}


