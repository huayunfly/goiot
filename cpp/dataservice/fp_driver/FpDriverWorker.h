#pragma once

// @purpose: FpDriverWorker for the detail driver action.
// @author: huayunfly at 126.com
// @date: 2024.10.11
// @copyright: GNU
// @version: 0.1

#include <string>
#include <memory>
#include <map>
#include "../dataservice/driver_base.h"
#include "../dataservice/ThreadSafeQueue.h"


namespace goiot
{
	class FpDriverWorker
	{
	public:
		FpDriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map,
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue, const std::string& driver_id) :
			_connection_details(connection_details), _driver_manager_reponse_queue(reponse_queue), _data_map(data_map),
			/*_connection_manager(), */_in_queue(10), _out_queue(10), _refresh(false), _driver_id(driver_id)
		{
			
		}

		FpDriverWorker(const FpDriverWorker&) = delete;
		FpDriverWorker& operator=(const FpDriverWorker&) = delete;

	private:
		std::once_flag _connection_init_flag;
		ConnectionInfo _connection_details;
		std::map<std::string, DataInfo> _data_map;
		//std::shared_ptr<sqlite3> _connection_manager;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> _driver_manager_reponse_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _in_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _out_queue;
		std::vector<std::thread> _threads;
		bool _refresh;
		std::string _driver_id;
	};
}