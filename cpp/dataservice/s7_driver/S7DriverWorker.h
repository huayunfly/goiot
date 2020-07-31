// S7 driver worker
// 2020.07.29, huayunfly at 126.com
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
			connection_manager_(), in_queue_(10), out_queue_(10), refresh_(false), connected_(false),
			db_mapping_()
		{
			ParseDBOffset(); // Uses db_start_offset_ 512 as a start value.
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
		void AsyncRead(const std::vector<DataInfo>& data_info_vec, int trans_id);
		// Puts asynchronous write request to the out_queue.
		void AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id);

	private:
		/// <summary>
		/// Read data from device.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>A DataInfo object with value or error_code.</returns>
		std::shared_ptr<DataInfo> ReadData(const DataInfo& data_info);

		/// <summary>
		/// Read batch data block and decouple them into vector. Change the input parameter and return DataInfo pointer.
		/// </summary>
		/// <param name="data_info_vec">DataInfo vector.</param>
		/// <returns>Changed dataInfo vector with return_code.</returns>
		std::shared_ptr<std::vector<DataInfo>> ReadBatchData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);

		/// <summary>
		/// Write data to device.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>Return 0 if writing succeeded. otherwise returns std::error_code.</returns>
		int WriteData(const DataInfo& data_info);

		/// <summary>
		/// Write multiple data to device. Change input DataInfo return_code and data_flow directly.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>Changed dataInfo vector with return_code.</returns>
		std::shared_ptr<std::vector<DataInfo>> WriteData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);

		/// <summary>
		/// Get number of byte used to read PLC data block. 
		/// </summary>
		/// <param name="datatype">Datatype</param>
		/// <returns>Byte number. For bit, always return a byte.</returns>
		int GetNumberOfByte(DataType datatype); 

		/// <summary>
		/// Get a float from 4 bytes.
		/// </summary>
		/// <param name="src">Byte array</param>
		/// <param name="pos">Start position</param>
		/// <returns>Float value</returns>
		float GetFloat(const std::vector<byte>& src, std::size_t pos);

		/// <summary>
		/// Get an int value from 4 bytes.
		/// </summary>
		/// <param name="src">Byte array</param>
		/// <param name="pos">Start position</param>
		/// <returns>Int value</returns>
		int GetInt(const std::vector<byte>& src, std::size_t pos);

		/// <summary>
		/// Get a short integer from 4 bytes.
		/// </summary>
		/// <param name="src">Byte array</param>
		/// <param name="pos">Start position</param>
		/// <returns>Int16 value</returns>
		int16_t GetInt16(const std::vector<byte> src, std::size_t pos);


		// Parse data block start and end offset for read optimizaiton.
		void ParseDBOffset();

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
		// S7 optimization
		std::map<int/* db_id */, std::pair<uint64_t/* db_start_offset_ */, uint64_t/* db_end_offset_ */>> db_mapping_;
	};
}


