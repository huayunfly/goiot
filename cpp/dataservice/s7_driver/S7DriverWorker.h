#pragma once

#include <string>
#include <memory>
#include <map>

#include "snap7.h"
#include "../dataservice/driver_base.h"
#include "../dataservice/ThreadSafeQueue.h"

namespace goiot
{
	enum class S7CPUStatus
	{
		Run,
		Stop,
		Unknown
	};

	class S7DriverWorker
	{
	public:
		S7DriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map,
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue) :
			connection_details_(connection_details), driver_manager_reponse_queue_(reponse_queue), data_map_(data_map),
			connection_manager_(), in_queue_(10), out_queue_(10), refresh_(false), connected_(false)
		{

		}

		S7DriverWorker(const S7DriverWorker&) = delete;
		S7DriverWorker& operator=(const S7DriverWorker&) = delete;
		int OpenConnection();
		void CloseConnection();
		// Spawns working threads.
		void Start();
		// Stops and waits the working threads completed.
		void Stop();
		// Refreshs the data reading or writing requests into in_queue.
		void Refresh();
		// Dispatch worker deals with the in_queue request, which may read/write message to hardware.
		// The return data are put into the out_queue.
		void Request_Dispatch();
		// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
		void Response_Dispatch();
		// Puts asynchronous read request to the in_queue.
		void AsyncRead(const std::vector<std::string> var_names,
			const std::vector<std::string> var_ids, int trans_id);
		// Puts asynchronous write request to the out_queue.
		void AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id);

	private:
		// Get PLC status
		S7CPUStatus PLCStatus();
		// List PLC blocks
		void ListBlocks();

	private:
		std::once_flag connection_init_flag_;
		ConnectionInfo connection_details_;
		std::map<std::string, DataInfo> data_map_;
		std::shared_ptr<TS7Client> connection_manager_;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> driver_manager_reponse_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> in_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> out_queue_;
		std::vector<std::thread> threads_;
		bool refresh_;
		bool connected_;
	};
}


