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
			connection_manager_(), in_queue_(10), out_queue_(10), refresh_(false), connected_(false)
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
		void AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id);

	private:
		/// <summary>
		/// Read data from device.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>A DataInfo object with value or error_code.</returns>
		std::shared_ptr<DataInfo> ReadData(const DataInfo& data_info);

		/// <summary>
		/// Read multi data from device.
		/// </summary>
		/// <param name="data_info">DataInfo object vector.</param>
		/// <returns>DataInfo object vector value or error_code.</returns>
		std::shared_ptr<std::vector<DataInfo>> ReadData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);

		/// <summary>
		/// Write data to device.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>Return 0 if writing succeeded. otherwise returns std::error_code.</returns>
		int WriteData(const DataInfo& data_info);

		/// <summary>
		/// Gets the number of registers, such as DWORD, float etc.
		/// </summary>
		/// <param name="datatype">DataType</param>
		/// <returns>The number</returns>
		int GetNumberOfRegister(DataType datatype);

		/// <summary>
		/// Assigns the modbus register value to the data value according to the date type.
		/// Considering the raito. 考虑转换因子
		/// </summary>
		/// <param name="data_info">A DataInfo object</param>
		/// <param name="registers">Modbus registers</param>
		void AssignRegisterValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint16_t> registers);

		/// <summary>
		/// Assigns the modbus register value to multi data value according to the date type.
		/// Copy the DataInfo and result only if the result != 0 (error)
		/// Considering the raito. 考虑转换因子
		/// </summary>
		/// <param name="rp_data_info_vec">A response DataInfo object vector</param>
		/// <param name="data_info_map">DataInfo map</param>
		/// <param name="register_start">register start</param>
		/// <param name="registers">Modbus registers</param>
		/// <param name="result">0: parse and copy registers; otherwise: copy DataInfo with error result only.</param>
		void AssignRegisterValue(std::shared_ptr<std::vector<DataInfo>> rp_data_info_vec,
			const std::map<int/*register*/, DataInfo>& data_info_map, int register_start, const std::vector<uint16_t>& registers, int result);

		/// <summary>
		/// Assigns the modbus bit value to the data value.
		/// </summary>
		/// <param name="data_info">A DataInfo object</param>
		/// <param name="bits">Modbus bits (1 bit in 1 unit8_t converted by libmodbus)</param>
		void AssignBitValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint8_t> bits);

		/// <summary>
		/// Assigns multi modbus bit value to the data values. If result != 0, copy DataInfo with errro result only.
		/// </summary>
		/// <param name="rp_data_info_vec">A response DataInfo object vector</param>
		/// <param name="data_info_map">A data map with key:register and value:DataInfo</param>
		/// <param name="register_start">register start</param>
		/// <param name="bits">Modbus bits (1 bit in 1 unit8_t converted by libmodbus)</param>
		/// <param name="result">0: parse and copy bits; otherwise: copy DataInfo with error result only.</param>
		void AssignBitValue(std::shared_ptr<std::vector<DataInfo>> rp_data_info_vec,
			const std::map<int/*register*/, DataInfo>& data_info_map, int register_start, const std::vector<uint8_t>& bits, 
			int result);

		/// <summary>
		/// Get register value from the given DataInfo object.
		/// Considering the ratio. 考虑转换因子
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>Register array.</returns>
		std::shared_ptr<uint16_t> GetRegisterValue(const DataInfo& data_info);

		/// <summary>
		/// Get bit value from the given DataInfo object.
		/// </summary>
		/// <param name="data_info">DataInfo object.</param>
		/// <returns>data ON: 0xFF00, OFF: 0x0000</returns>
		uint16_t GetBitValue(const DataInfo& data_info);

		/// <summary>
		/// Judge whether the writing data equals the modbus_write_and_read() response data.
		/// </summary>
		/// <param name="data_info">The writeing data_info.</param>
		/// <param name="registers">Read response registers.</param>
		/// <returns>True if they are equal, otherwise false.</returns>
		bool DataInfoValueEqualsReadValue(const DataInfo& data_info, std::shared_ptr<uint16_t> registers);

	private:
		const int REFRESH_INTERVAL = 500; // in ms

		std::once_flag connection_init_flag_;
		ConnectionInfo connection_details_;
		std::map<std::string, DataInfo> data_map_;
		std::shared_ptr<modbus_t> connection_manager_;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> driver_manager_reponse_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> in_queue_;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> out_queue_;
		std::vector<std::thread> threads_;
		bool refresh_;
		bool connected_;
	};
}


