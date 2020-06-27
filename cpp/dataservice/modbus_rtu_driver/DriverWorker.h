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
		DriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map,
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue) :
			connection_details_(connection_details), driver_manager_reponse_queue_(reponse_queue), data_map_(data_map),
			connection_manager_(), in_queue_(10), out_queue_(10), refresh_(false)
		{
			
		}
		DriverWorker(const DriverWorker&) = delete;
		DriverWorker& operator=(const DriverWorker&) = delete;
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
		void AsyncWrite(const std::vector<DataInfo>& data_info, int trans_id);

	private:
		std::shared_ptr<DataInfo> ReadData(const DataInfo& data_info);
		std::shared_ptr<DataInfo> WriteData(const DataInfo& data_info);

		/// <summary>
		/// Gets the number of registers, such as DWORD, float etc.
		/// </summary>
		/// <param name="datatype">DataType</param>
		/// <returns>The number</returns>
		int GetNumberOfRegister(DataType datatype);

		/// <summary>
		/// Assigns the modbus register value to the data value according to the date type.
		/// </summary>
		/// <param name="data_info">A DataInfo object</param>
		/// <param name="registers">Modbus registers</param>
		void AssignRegisterValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint16_t> registers);

		/// <summary>
		/// Assigns the modbus bit value to the data value.
		/// </summary>
		/// <param name="data_info">A DataInfo object</param>
		/// <param name="bits">Modbus bits (1 bit in 1 unit8_t converted by libmodbus)</param>
		void AssignBitValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint8_t> bits);


	private:
		std::once_flag connection_init_flag_;
		ConnectionInfo connection_details_;
		std::map<std::string, DataInfo> data_map_;
		std::shared_ptr<modbus_t> connection_manager_;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> driver_manager_reponse_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> in_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> out_queue_;
		std::vector<std::thread> threads_;
		bool refresh_;
	};
}


