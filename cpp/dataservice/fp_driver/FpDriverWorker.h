#pragma once

// @purpose: FpDriverWorker for the detail driver action.
// @author: huayunfly at 126.com
// @date: 2024.10.11
// @copyright: GNU
// @version: 0.1

#include <string>
#include <memory>
#include <map>
#include <boost/asio.hpp>
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
			/*_connection_manager(), */_io_ctx(std::make_unique<boost::asio::io_context>()), _in_queue(10), _out_queue(10), _refresh(false), _connected(false), _driver_id(driver_id)
		{

			
		}

		FpDriverWorker(const FpDriverWorker&) = delete;
		FpDriverWorker& operator=(const FpDriverWorker&) = delete;
		int OpenConnection();
		void CloseConnection();
		// Spawns working threads.
		void Start();
		// Stops and waits the working threads completed.
		void Stop();
		// Refreshs the data reading or writing requests into in_queue, classified by DF(float), R(register), DW(dword) types. 
		void Refresh();
		// Dispatch worker deals with the in_queue request, which may read/write message to hardware.
        // The return data are put into the out_queue.
		void Request_Dispatch();
		// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
		void Response_Dispatch();
		// Read fp2 plc multi-type data 
		std::shared_ptr<std::vector<DataInfo>> ReadMultiTypeData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);
		// Write data to fp2 plc
		std::shared_ptr<std::vector<DataInfo>> WriteData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);
		// Int to BCD in range[0, 99999]
		int Int2BCD(int val);
		// Positive Int to ASCII string with prefixed "0" 
		std::string UInt2ASCIIWithFixedDigits(int val, int digits);

	private:
		std::once_flag _connection_init_flag;
		ConnectionInfo _connection_details;
		std::map<std::string, DataInfo> _data_map;
		std::shared_ptr<boost::asio::serial_port> _connection_manager;
		std::unique_ptr<boost::asio::io_context> _io_ctx;
		std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> _driver_manager_reponse_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _in_queue;
		ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>> _out_queue;
		std::vector<std::thread> _threads;
		bool _refresh;
		bool _connected;
		std::string _driver_id;
	};
}