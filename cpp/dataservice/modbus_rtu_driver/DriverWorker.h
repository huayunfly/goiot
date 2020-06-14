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
#include "../dataservice/ThreadSafeQueue.h"

namespace goiot
{
	class DriverWorker
	{
	public:
		DriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map) :
			connection_details_(connection_details), data_map_(data_map), connection_manager_(), 
			in_queue_(10), out_queue_(10), refresh_(false)
		{

		}
		DriverWorker(const DriverWorker&) = delete;
		DriverWorker& operator=(const DriverWorker&) = delete;
		int OpenConnection();
		void CloseConnection();
		void Start();
		void Stop();
		void Refresh();
		void Request_Dispatch();
		void Response_Dispatch();
		void AsyncRead(const std::vector<std::string> var_names, 
			const std::vector<std::string> var_ids, int trans_id);
		void AsyncWrite(const std::vector<DataInfo>& data_info, int trans_id);

	private:
		std::once_flag connection_init_flag_;
		ConnectionInfo connection_details_;
		std::map<std::string, DataInfo> data_map_;
		std::shared_ptr<modbus_t> connection_manager_;
		ThreadSafeQueue<DataInfo> in_queue_;
		ThreadSafeQueue<DataInfo> out_queue_;
		std::vector<std::thread> threads_;
		bool refresh_;
	};
}


