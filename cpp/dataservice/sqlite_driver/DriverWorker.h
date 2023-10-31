#pragma once

// @purpose: DriverWorker for the detail driver action.
// @author: huayunfly at 126.com
// @date: 2023.10.30
// @copyright: GNU
// @version: 0.1

#include <string>
#include <memory>
#include <map>
#include "sqlite3.h"
#include "../dataservice/driver_base.h"
#include "../dataservice/ThreadSafeQueue.h"


namespace goiot
{
	class SqliteDriverWorker
	{
	public:
		SqliteDriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map,
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue) :
			_connection_details(connection_details), _driver_manager_reponse_queue(reponse_queue), _data_map(data_map),
			_connection_manager(), _in_queue(10), _out_queue(10), _refresh(false)
		{

		}
		SqliteDriverWorker(const SqliteDriverWorker&) = delete;
		SqliteDriverWorker& operator=(const SqliteDriverWorker&) = delete;

		int OpenConnection();
		void CloseConnection();
		// Spawns working threads.
		void Start();
		// Stops and waits the working threads completed.
		void Stop();
		// Refreshs the data reading or writing requests into in_queue.
		void Refresh();

	private:
		std::once_flag _connection_init_flag;
		ConnectionInfo _connection_details;
		std::map<std::string, DataInfo> _data_map;
		std::shared_ptr<sqlite3> _connection_manager;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> _driver_manager_reponse_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _in_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _out_queue;
		std::vector<std::thread> _threads;
		bool _refresh;
	};
}


