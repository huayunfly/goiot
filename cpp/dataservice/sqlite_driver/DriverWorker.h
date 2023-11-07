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
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue, const std::string& driver_id) :
			_connection_details(connection_details), _driver_manager_reponse_queue(reponse_queue), _data_map(data_map),
			_connection_manager(), _in_queue(10), _out_queue(10), _refresh(false), _driver_id(driver_id)
		{
			DataInfo2DBTableInfo(_data_map, _table_info);
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
		// Dispatch worker deals with the in_queue request, which may read/write message to hardware.
		// The return data are put into the out_queue.
		void Request_Dispatch();
		// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
		void Response_Dispatch();
		// Puts asynchronous read request to the in_queue.
		void AsyncRead(const std::vector<DataInfo>& data_info_vec, int trans_id);
		// Puts asynchronous write request to the out_queue. Return non zero if the queue is full.
		int AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id);
		// Retrieve DB table column names from DataInfo collection.
		void DataInfo2DBTableInfo(const std::map<std::string, DataInfo> data_map, 
			std::map<std::string, std::list<std::string>>& db_table_info);
		// Retriveve DB table column pair<data_id, table_column_id> form data map
		void DataInfo2DBTableInfo(const std::map<std::string, std::string> data_map,
			std::map<std::string, std::list<std::pair<std::string, std::string>>>& db_table_info);
		// Create table if it does not exist. Trim table columns according to DataInfo.
		void CreateOrTrimDBTable();
		// sqlite3 callback function.
		static int SqlCallback(void* data, int argc, char** argv, char** col_name);
		// clear cache
		static void ClearGlobalDataCache()
		{
			_global_data_cache.clear();
		}

		/// <summary>
		/// Read multiple DB tables' data into vector. Change the input parameter and return DataInfo pointer.
		/// </summary>
		/// <param name="data_info_vec">DataInfo vector.</param>
		/// <returns>Changed dataInfo vector with return_code.</returns>
		std::shared_ptr<std::vector<DataInfo>> ReadMultiTableData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);
		/// <summary>
		/// Write multiple data to multiple tables in DB. Change input DataInfo return_code and data_flow directly.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>Changed dataInfo vector with return_code.</returns>
		std::shared_ptr<std::vector<DataInfo>> WriteData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);

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
		std::string _driver_id;
		// sqlite specified table info <table_name, [column names]>
		std::map<std::string, std::list<std::string>> _table_info;
		// static member, not thread-safe!!!
		static std::map<std::string, std::string> _global_data_cache;
		static std::string NULL_STRING;
	};
}


