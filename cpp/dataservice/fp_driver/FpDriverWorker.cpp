#include "pch.h"
#include "FpDriverWorker.h"
#include <iostream>
#include <iomanip>


namespace goiot
{
	int FpDriverWorker::OpenConnection()
	{

		int bcd = UInt2BCD(99998123);

		std::string req = "%01#RDD";

		std::string bcd1 = BCCStr2BCDStr("%01#RDD0066000661");

		_connection_manager.reset(new boost::asio::serial_port(*_io_ctx));

		boost::system::error_code ec;
		_connection_manager->open(_connection_details.port, ec);
		if (ec)
		{
			_connected = false;
			std::cout << "FpDriver Connection failed: " << ec.message() << std::endl;
			return ENOTCONN;
		}

		_connection_manager->set_option(boost::asio::serial_port_base::baud_rate(_connection_details.baud));
		switch (_connection_details.parity)
		{
		case 'O':
			_connection_manager->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd));
			break;
		case 'E':
			_connection_manager->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even));
			break;
		default:
			_connection_manager->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
		}	
		_connection_manager->set_option(boost::asio::serial_port_base::character_size(_connection_details.data_bit));
		if (_connection_details.stop_bit == 2)
		{
			_connection_manager->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two));
		}
		else
		{
			_connection_manager->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
		}
		_connected = true;
		return 0;
	}

	void FpDriverWorker::CloseConnection()
	{
		_connection_manager->close();
		_connection_manager.reset();
		_connected = false;
	}

	void FpDriverWorker::Start()
	{
		std::call_once(_connection_init_flag, &FpDriverWorker::OpenConnection, this);
		_refresh = true;
		_threads.emplace_back(&FpDriverWorker::IORun, this);
		_threads.emplace_back(&FpDriverWorker::Refresh, this);
		_threads.emplace_back(&FpDriverWorker::Request_Dispatch, this);
		_threads.emplace_back(&FpDriverWorker::Response_Dispatch, this);
	}

	void FpDriverWorker::Stop()
	{
		_io_ctx->stop();
		_refresh = false;
		_in_queue.Close();
		_out_queue.Close();
		for (auto& entry : _threads)
		{
			entry.join();
		}
	}

	void FpDriverWorker::IORun()
	{
		_io_ctx->run();
	}

	// Refreshs the data reading or writing requests into in_queue, classied by DF(float), R(register), DW(dword) types. 
	void FpDriverWorker::Refresh()
	{
		const int TYPE_INDEX_BT = 0;
		const int TYPE_INDEX_DB = 1;
		const int TYPE_INDEX_DF = 2;
		const int TYPE_NUM = 3;

		while (_refresh)
		{
			try
			{
				std::vector<std::vector<DataInfo>> data_info_vecs(TYPE_NUM);
				for (auto data_info : _data_map)
				{
					if (data_info.second.read_write_priviledge != ReadWritePrivilege::WRITE_ONLY)
					{
						int type_index = -1;
						switch (data_info.second.data_type)
						{
						case DataType::BT:
							type_index = TYPE_INDEX_BT;
							break;
						case DataType::DB:
							type_index = TYPE_INDEX_DB;
							break;
						case DataType::DF:
							type_index = TYPE_INDEX_DF;
							break;
						default:
							break;
						}
						if (type_index < 0)
						{
							continue; // Skip unsupported data types.
						}
						data_info_vecs.at(type_index).emplace_back(data_info.second.id,
							data_info.second.name, data_info.second.address, data_info.second.register_address,
							data_info.second.read_write_priviledge, DataFlowType::REFRESH, data_info.second.data_type,
							data_info.second.data_zone, data_info.second.float_decode, data_info.second.dword_decode, data_info.second.byte_value, data_info.second.int_value, data_info.second.float_value,
							data_info.second.char_value, std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0);
					}
				}
				for (auto& vec : data_info_vecs)
				{
					if (vec.size() > 0)
					{
						_in_queue.Put(std::make_shared<std::vector<DataInfo>>(vec), true, std::chrono::milliseconds(1000));
					}			
				}
			}
			catch (const QFull&)
			{
				std::cerr << "fpdriver:In-queue is full" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "fpdriver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "fpdriver:EXCEPTION (unknown)" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(_connection_details.refresh_interval_msec));
		}
	}

	void FpDriverWorker::Request_Dispatch()
	{
		while (true)
		{
			try
			{
				std::shared_ptr<std::vector<DataInfo>> data_info_vec;
				_in_queue.Get(data_info_vec);
				if (data_info_vec == nullptr) // Improve for a robust SENTINEL
				{
					break; // Exit
				}
				std::shared_ptr<std::vector<DataInfo>> rp_data_info_vec;
				int result_code = 0;

				if (data_info_vec->empty())
				{
					continue;
				}
				if (data_info_vec->at(0).data_flow_type == DataFlowType::REFRESH)
				{
					rp_data_info_vec = ReadMultiTypeData(data_info_vec); // Modify data_info_vec directly, may be improve.
				}
				else if (data_info_vec->at(0).data_flow_type == DataFlowType::ASYNC_WRITE)
				{
					rp_data_info_vec = WriteData(data_info_vec);
				}
				else
				{
					throw std::invalid_argument("Unsupported data flow");
				}
				_out_queue.Put(rp_data_info_vec, true, std::chrono::milliseconds(1000));
			}
			catch (const QFull&)
			{
				std::cerr << "fpplc_driver:Out-queue is full" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "fpplc_driver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "fpplc_driver:EXCEPTION (unknown)" << std::endl;
			}
		}
	}

	void FpDriverWorker::Response_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			_out_queue.Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			try
			{
				_driver_manager_reponse_queue->PutNoWait(data_info_vec);
			}
			catch (const QFull&)
			{
				assert(false);
				std::cout << "sqlite_driver:driver_manager_response_queue_ is full." << std::endl;
			}
		}
	}

	std::shared_ptr<std::vector<DataInfo>> FpDriverWorker::ReadMultiTypeData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		const char END_OF_CMD = '\r';
		const std::string REPLY_HEAD = "%01$";
		const std::size_t TYPE_INDEX_BT = 0;
		const std::size_t TYPE_INDEX_DB = 1;
		const std::size_t TYPE_INDEX_DF = 2;
		const int START_LIMIT = 99999;
		const int END_LIMIT = -1;

		std::pair<int, int> data_block_range(std::make_pair(START_LIMIT, END_LIMIT));
		for (std::size_t i = 0; i < data_info_vec->size(); i++)
		{
			if (data_info_vec->at(i).register_address < data_block_range.first)
			{
				data_block_range.first = data_info_vec->at(i).register_address;
			}
			else if (data_info_vec->at(i).register_address > data_block_range.second)
			{
				data_block_range.second = data_info_vec->at(i).register_address;
			}
		}
		if (data_block_range.first > data_block_range.second)
		{
			for (auto& data_info : *data_info_vec)
			{
				data_info.result = EINVAL;
				data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
			}
			return data_info_vec; // Mark invalid and return.
		}

		if (_connection_manager->is_open())
		{
			std::string req;
			switch (data_info_vec->at(0).data_type)
			{
			case DataType::BT:
				req = "%01#RCC";
				break;
			case DataType::DB:
				req = "%01#RDB";
				break;
			case DataType::DF:
				// 发送 % 01 # RD D 00660 00661 54 \r(回车)
				// 回复 % 01$RD9A99B94110  解释为浮点数 41B9 999A 即23.200000762939453
				req = "%01#RDD";
				req += UInt2ASCIIWithFixedDigits(data_block_range.first, 5);
				if (data_block_range.first == data_block_range.second)
				{		
					req += UInt2ASCIIWithFixedDigits(data_block_range.first + 1, 5); // double value: address + 1
				}
				else
				{
					req += UInt2ASCIIWithFixedDigits(data_block_range.second + 1, 5); // double value: address + 1
				}
				req += BCCStr2BCDStr(req);
				req += END_OF_CMD;
				try
				{
					// async write
					boost::asio::write(*_connection_manager, boost::asio::buffer(req));
					//auto output_buffer = boost::asio::buffer(req);
					//boost::asio::async_write(*_connection_manager,
					//	output_buffer,
					//	[](boost::system::error_code ec, std::size_t sz)
					//	{
					//		std::cout << "Size Written " << sz << " Ec: "
					//			<< ec << ec.message() << '\n';
					//	});
					boost::system::error_code ec;
					boost::asio::streambuf input_buffer;
					// Sync read, return 0 if an error occurred.
					size_t len = boost::asio::read_until(*_connection_manager, input_buffer, END_OF_CMD, ec);
					if (len > 0)
					{
						std::string reply((std::istreambuf_iterator<char>(&input_buffer)), std::istreambuf_iterator<char>());
						input_buffer.consume(len); // Drop the buffer data.
						reply.erase(reply.end() - 1); // Remove the tail char "END_OF_CMD".
						std::cout << reply << std::endl;
						const std::string RDD_HEAD = "%01$";
						if (reply.size() <= REPLY_HEAD.size() || 
							reply.substr(0, REPLY_HEAD.size()).compare(REPLY_HEAD) != 0)
						{
							throw std::invalid_argument("fpplc::RDD reply's head is error.");
						}
						// BCC check
						std::string tail = reply.substr(reply.size() - 2, 2);
						std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
						if (tail.compare(bcc) != 0)
						{
							throw std::invalid_argument("fpplc::RDD reply's BCC checking is error.");
						}
						std::string data_str = 
							reply.substr(REPLY_HEAD.size(), reply.size() - REPLY_HEAD.size() - 2);
						int data_count = (data_block_range.second == data_block_range.first) 
							? 1 : ((data_block_range.second - data_block_range.first) / 2 + 1);
						if ((data_str.size() / 8/*double*/) != data_count)
						{
							throw std::invalid_argument("fpplc::RDD reply's data count is error.");
						}
					}

                    // async read
					//boost::asio::deadline_timer read_timer(*_io_ctx);
					//read_timer.expires_from_now(boost::posix_time::seconds(0));
					//read_timer.async_wait(std::bind(&FpDriverWorker::handle_timeout, this, std::placeholders::_1));
					//boost::asio::streambuf buf;
					//boost::asio::async_read_until(*_connection_manager, _input_buffer, END_OF_CMD,
					//	std::bind(&FpDriverWorker::handle_read, this, std::placeholders::_1, std::placeholders::_2)
						//[&buf](boost::system::error_code ec, std::size_t sz)
						//{
						//	std::string s((std::istreambuf_iterator<char>(&_input_buffer)), std::istreambuf_iterator<char>());
						//	_input_buffer.consume(sz);
						//	std::cout << "Msg: " << s << " size " << s.size() << " Size " <<
						//		sz << " " << ec << ec.message() << '\n';

						//	//std::string reply_str(receive_buffer);
						//	//if (len <= 4 || reply_str.substr(0, 4) != "%01$")
						//	//{
						//	//	throw std::invalid_argument("Communication error.");
						//	//}
						//	//int data_count = ((data_block_range.second - data_block_range.first) + 1) * 2; // double value
						//	//// BCC check
						//	//if (reply_str.substr(len - 1, 1) != BCCStr2BCDStr(reply_str.substr(9, len - 1)))
						//	//{
						//	//	throw std::invalid_argument("BCC error");
						//	//}
						//
						//);
				}
				catch (boost::system::system_error& e)
				{
					boost::system::error_code ec = e.code();
					std::cerr << ec.value() << std::endl;
					std::cerr << ec.category().name() << std::endl;
				}
				break;
			default:
				throw std::runtime_error("Unsupported fp2 data type.");
			}		
		}
		//IORun();
		for (auto& data_info : *data_info_vec)
		{
			data_info.result = EINVAL;
			data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
		}
		return data_info_vec; // Mark invalid and return.
	}

	std::shared_ptr<std::vector<DataInfo>> FpDriverWorker::WriteData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (_connection_manager)
		{
			return nullptr;
		}
	}

	int FpDriverWorker::UInt2BCD(int val)
	{
		if (val < 0)
		{
			return 0;
		}

		int bcd = 0;
		std::string numstr = std::to_string(val);
		if (numstr.size() > 5)
		{
			return 0; // Overflow protection.
		}
		int offset = 0;
		for (char c : numstr)
		{
			bcd = bcd << 4;
			int num = c - '0';
			bcd += num;		
		}
		return bcd;
	}

	std::string FpDriverWorker::UInt2ASCIIWithFixedDigits(int val, int digits)
	{
		// 998 -> "998"
		if (val < 0)
		{
			return "";
		}
		std::stringstream sstr;
		sstr << std::setw(digits) << std::setfill('0') << val;
		return sstr.str();
	}

	std::string FpDriverWorker::BCCStr2BCDStr(const std::string& val)
	{
		// "%01#RDD0066000661" (BCC->) 0x54
		char bcc = 0;
		bool first = true;
		for (char c : val)
		{
			if (first)
			{
				bcc = c;
				first = false;
			}
			else
			{
				bcc ^= c;
			}
		}		
		return std::to_string((bcc >> 4) & 0x0F) + std::to_string(bcc & 0x0F);
	}
	
	void FpDriverWorker::handle_read(const boost::system::error_code& error, 
		std::size_t bytes_transferred) 
	{
		if (error) {
			std::cerr << "Read error: " << error.message() << std::endl;
			return;
		}
		std::string str((std::istreambuf_iterator<char>(&_input_buffer)), std::istreambuf_iterator<char>());
        _input_buffer.consume(bytes_transferred); // Drops data in the stream buffer after the data processing.
		std::cout << str << std::endl;


	}

	void FpDriverWorker::handle_timeout(const boost::system::error_code& error) 
	{
		if (error == boost::asio::error::operation_aborted) 
		{
			return;
		}
		std::cerr << "Operation timed out!" << std::endl;
	}
}

