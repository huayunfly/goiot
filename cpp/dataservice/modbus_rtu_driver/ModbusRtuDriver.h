#pragma once

#include <iostream>
#include <system_error>
#include <cassert>
#include <ctime>
#include <json/json.h>
#include "../dataservice/driver_base.h"
#include "DriverWorker.h"

namespace goiot
{
	class ModbusRtuDriver : public DriverBase
	{
	public:
		ModbusRtuDriver() : driver_worker_(), id_(), worker_ready_(false)
		{

		}

		ModbusRtuDriver(const ModbusRtuDriver&) = delete;
		ModbusRtuDriver& operator=(const ModbusRtuDriver&) = delete;

		RESULT_DSAPI GetDescription(std::string& description)
		{
			description = "Modbus_Rtu";
			return 0;
		}

		virtual RESULT_DSAPI GetID(std::string& id)
		{
			id = id_;
			return 0;
		}

		RESULT_DSAPI InitDriver(const std::string& config, 
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue,
			std::function<void(const DataInfo&)> set_data_info);

		RESULT_DSAPI UnitDriver();

		virtual RESULT_DSAPI AsyncWrite(const std::vector<DataInfo>& data_info_vec);

		virtual RESULT_DSAPI AsyncRead(const std::vector<std::string> id_vec,
			std::function<void(std::vector<DataInfo>&&)> read_callback);

	private:
		/// <summary>
		/// Parse driver ID, device data_info, connection string.
		/// </summary>
		/// <param name="config">Configuration json</param>
		/// <param name="connection_info">ConnectionInfo</param>
		/// <param name="data_map">A data map reference.</param>
		/// <returns>0: succeeded, otherwise failed.</returns>
		int ParseConfig(const std::string& config,
			ConnectionInfo& connection_info, std::map<std::string, DataInfo>& data_map);

	private:
		std::unique_ptr<DriverWorker> driver_worker_;
		std::string id_;
		bool worker_ready_;
	};
}

