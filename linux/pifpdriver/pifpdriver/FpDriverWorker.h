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
#include "../../pidataservice/pidataservice/driver_base.h"
#include "../../pidataservice/pidataservice/ThreadSafeQueue.h"


namespace goiot
{
	class FpDriverWorker
	{
	public:
		FpDriverWorker(const ConnectionInfo& connection_details, std::map<std::string, DataInfo>&& data_map,
			std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> reponse_queue, const std::string& driver_id) :
			_connection_details(connection_details), _driver_manager_reponse_queue(reponse_queue), _data_map(data_map),
			/*_connection_manager(), */_io_ctx(std::make_unique<boost::asio::io_context>()), _in_queue(10), _out_queue(10), _refresh(false), _connected(false)
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
		// Puts asynchronous write request to the out_queue. Return non zero if the queue is full.
		int AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id);
		// Read fp2 plc multi-type data 
		std::shared_ptr<std::vector<DataInfo>> ReadMultiTypeData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);
		// Write data to fp2 plc
		std::shared_ptr<std::vector<DataInfo>> WriteData(std::shared_ptr<std::vector<DataInfo>> data_info_vec);
		// Unsigned Int to ASCII string with prefixed "0" 
		std::string UInt2ASCIIWithFixedDigits(int val, int digits);
		// BCC string validation to BCD char
		std::string BCCStr2BCDStr(const std::string& val);
		// BCC string to float
		std::vector<float> BCDStr2Float(const std::string& data_str);
		// Float to BCD string
		std::string Float2BCDStr(const std::vector<float>& data_vec);
		// Word to BCD string
		std::string Word2BCDStr(const std::vector<uint16_t>& data_vec);
		// BCD string to word
		std::vector<unsigned short> BCDStr2Word(const std::string& data_str);
        // Unsigned integer to single BCD character.
		std::string UInt2SingleBCDChar(unsigned int value);
		// Set DataInfo result fvalue
		void SetDataInfoResult(DataInfo& data_info, float fvalue, DataFlowType data_flow_type, int result);
		// Set DataInfo result bool value
		void SetDataInfoResult(DataInfo& data_info, uint8_t bvalue, DataFlowType data_flow_type, int result);
		// Set DataInfo result ushort value
		void SetDataInfoResult(DataInfo& data_info, uint16_t svalue, DataFlowType data_flow_type, int result);
		// Set DataInfo result without value
		void SetDataInfoResult(DataInfo& data_info, DataFlowType data_flow_type, int result);
		// Test
		void Test();

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
	};
}