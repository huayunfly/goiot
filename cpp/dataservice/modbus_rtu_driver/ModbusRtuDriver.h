#pragma once

#include <iostream>
#include <system_error>
#include <cassert>
#include <json/json.h>
#include "../dataservice/driver_base.h"
#include "DriverWorker.h"

namespace goiot
{
	class ModbusRtuDriver : public DriverBase
	{
	public:
		ModbusRtuDriver() : driver_worker_()
		{

		}

		ModbusRtuDriver(const ModbusRtuDriver&) = delete;
		ModbusRtuDriver& operator=(const ModbusRtuDriver&) = delete;

		RESULT_DSAPI GetDescription(std::string& description)
		{
			description = "Modbus_Rtu";
			return 0;
		}

		RESULT_DSAPI InitDriver(const std::string& config);

		RESULT_DSAPI UnitDriver();

	private:
		int ParseConfig(const std::string& config,
			ConnectionInfo connection_info, std::map<std::string, DataInfo> data_map);

	private:
		std::shared_ptr<DriverWorker> driver_worker_;
	};
}

