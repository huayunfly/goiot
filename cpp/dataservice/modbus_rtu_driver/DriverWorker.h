#pragma once

// @purpose: DriverWorker for the detail driver action.
// @author: huayunfly@126.com
// @date: 2020.06.06
// @copyright: GNU
// @version: 0.1

#include <string>
#include <memory>
#include <map>

#include "modbus.h"
#include "../dataservice/driver_base.h"

namespace goiot
{
	class DriverWorker
	{
	public:
		DriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map) : 
			connection_details_(connection_details), data_map_(data_map), connection_manager_()
		{

		}
		DriverWorker(const DriverWorker&) = delete;
		DriverWorker& operator=(const DriverWorker&) = delete;
		int OpenConnection();
		void CloseConnection();
		void DoWork();

	private:
		std::once_flag connection_init_flag_;
		ConnectionInfo connection_details_;
		std::map<std::string, DataInfo> data_map_;
		std::shared_ptr<modbus_t> connection_manager_;
	};
}


