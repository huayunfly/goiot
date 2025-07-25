#include "FpDriverWorker.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include "BlockingReader.h"
#include "BlockingWriter.h"


namespace goiot
{
	int FpDriverWorker::OpenConnection()
	{
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
		_threads.emplace_back(&FpDriverWorker::Refresh, this);
		_threads.emplace_back(&FpDriverWorker::Request_Dispatch, this);
		_threads.emplace_back(&FpDriverWorker::Response_Dispatch, this);
	}

	void FpDriverWorker::Stop()
	{
		_refresh = false;
		_in_queue.Close();
		_out_queue.Close();
		for (auto& entry : _threads)
		{
			entry.join();
		}
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

	// Async write
	int FpDriverWorker::AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id)
	{
		if (data_info_vec.size() > 0)
		{
			assert(data_info_vec.at(0).data_flow_type == DataFlowType::ASYNC_WRITE);
			try
			{
				_in_queue.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec),
					true, std::chrono::milliseconds(200)); // prevent blocking DataService poll dispatch()
			}
			catch (const QFull&)
			{
#ifdef _DEBUG
				std::cout << "fpplc_driver in_queue_ is full." << std::endl;
#endif // _DEBUG
				return ETIMEDOUT;
			}
		}
		return 0;
	}

	std::shared_ptr<std::vector<DataInfo>> FpDriverWorker::ReadMultiTypeData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		// Pre-requisition: one serial port supports ONE panasonic fpplc, 
		// so all data_info.address is the same.
		const std::string ADDR = std::string("%") + UInt2ASCIIWithFixedDigits(data_info_vec->cbegin()->address, 2);

		const std::string END_OF_CMD = "\r";
		const std::string REPLY_READ_FLOAT_HEAD = ADDR + "$RD"; // "%01$RD" for example
		const std::string REPLY_READ_WORD_HEAD = ADDR + "$RD";
		const std::string REPLY_READ_REGISTER_HEAD = ADDR + "$RC";
		const std::size_t TYPE_INDEX_BT = 0;
		const std::size_t TYPE_INDEX_DB = 1;
		const std::size_t TYPE_INDEX_DF = 2;
		const int START_LIMIT = 99999;
		const int END_LIMIT = -1;
		const int MAX_READ_WORD_COUNT = 26;

		std::map<int, int> reg_addr_index;
		int i = 0;
		for (const auto& elem : *data_info_vec)
		{
			// Map insertion with default sort().
			reg_addr_index.insert({elem.register_address, i});
			i++;
		}
		int first = reg_addr_index.begin()->first;
		int last = reg_addr_index.rbegin()->first;

		// Divide into some batches, a batch contains max 26 WORDs.
		std::vector<std::pair<int, int>> data_block_range;
		int start = 0;
		int end = 0;
		int word_offset;
		switch (data_info_vec->at(0).data_type)
		{
		case DataType::BT:
			start = first / 16;
			end = last / 16;
			word_offset = 1;
			break;
		case DataType::WB:
			start = first;
			end = last;
			word_offset = 1;
			break;
		case DataType::DF:
			start = first;
			end = last;
			word_offset = 2;
			break;
		default:
			throw std::invalid_argument("Unsupported fpplc data type.");
		}
		while (true)
		{
			if (start + MAX_READ_WORD_COUNT <= end)
			{
				data_block_range.push_back(std::make_pair(start, start + MAX_READ_WORD_COUNT - word_offset));
				start += MAX_READ_WORD_COUNT;
			}
			else
			{
				data_block_range.push_back(std::make_pair(start, end));
				break;
			}
		}

		if (_connection_manager->is_open())
		{
			std::string req;
			switch (data_info_vec->at(0).data_type)
			{
			case DataType::BT:
				//	* 按字单位读取触点状态 （R寄存器），返回3C00解释为003C，按位取3C bit位。
				//	发送 % 01 # RCC R 0108 0112 0C \r
				//	回复 % 01$RC3C000000000000000C0012
				try
				{
					std::vector<unsigned short> total_values;
					std::vector<int> operation_results;
					for (auto& divided_range : data_block_range)
					{
						req = ADDR + "#RCCR";
						req += UInt2ASCIIWithFixedDigits(divided_range.first, 4);
						req += UInt2ASCIIWithFixedDigits(divided_range.second, 4);
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						int data_count = (divided_range.second == divided_range.first)
							? 1 : (divided_range.second - divided_range.first + 1);
						// Sync write
						//boost::asio::write(*_connection_manager, boost::asio::buffer(req));
					    // Block write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::RC write timeout or error." << std::endl;
#endif // DEBUG
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0);
							continue;
						}
						// Block read
						std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_READ_REGISTER_HEAD.size() ||
								reply.substr(0, REPLY_READ_REGISTER_HEAD.size()).compare(REPLY_READ_REGISTER_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RC reply's head is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RC reply's BCC checking is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							std::string data_str =
								reply.substr(REPLY_READ_REGISTER_HEAD.size(), reply.size() - REPLY_READ_REGISTER_HEAD.size() - 2/*BCC*/);
							if ((data_str.size() / 4/*word*/) != data_count)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RC reply's data count is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							operation_results.insert(operation_results.end(), data_count, 0/*S_OK*/);
							std::vector<unsigned short> values = BCDStr2Word(data_str);
							total_values.insert(total_values.end(), values.begin(), values.end());
						}
						else
						{
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0);
						}
					}
					for (auto& data_info : *data_info_vec)
					{
						std::size_t word_index = (data_info.register_address - first) / 16;
						unsigned short bit_index = data_info.register_address % 16;
						uint8_t byte_value = ((total_values.at(word_index) & (1 << bit_index))) > 0 ? 1 : 0;
						SetDataInfoResult(data_info, byte_value, DataFlowType::READ_RETURN, 
							operation_results.at(word_index));
					}
					return data_info_vec;
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			case DataType::WB:
				// 发送 % 01 # RD D 00580 00581 54 \r(回车)
				// 回复 % %01$RD0000000016  解释为2个字 值均为0000
				// 最大只能读26个字
				try
				{
					std::vector<uint16_t> total_values;
					std::vector<int> operation_results;
					for (auto& divided_range : data_block_range)
					{
						req = ADDR + "#RDD";
						req += UInt2ASCIIWithFixedDigits(divided_range.first, 5);
						req += UInt2ASCIIWithFixedDigits(divided_range.second, 5);
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						// Calculate the reading float data count.
						int data_count = (divided_range.second == divided_range.first)
							? 1 : (divided_range.second - divided_range.first + 1);
						// Block write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::RDD word write timeout or error." << std::endl;
#endif // DEBUG
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0);
							continue;
						}
						// Block read
						std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_READ_WORD_HEAD.size() ||
								reply.substr(0, REPLY_READ_WORD_HEAD.size()).compare(REPLY_READ_WORD_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD word reply's head is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD word reply's BCC checking is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							std::string data_str =
								reply.substr(REPLY_READ_WORD_HEAD.size(), reply.size() - REPLY_READ_WORD_HEAD.size() - 2/*BCC*/);
							if ((data_str.size() / 4/*word*/) != data_count)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD word reply's data count is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0);
								continue;
							}
							operation_results.insert(operation_results.end(), data_count, 0/*S_OK*/);
							std::vector<uint16_t> values = BCDStr2Word(data_str);
							total_values.insert(total_values.end(), values.begin(), values.end());
						}
						else
						{
#ifdef _DEBUG
							std::cerr << "fpplc::RDD reply timeout or error." << std::endl;
#endif // DEBUG
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0);
						}
					}
					for (auto& data_info : *data_info_vec)
					{
						std::size_t word_index = static_cast<std::size_t>(data_info.register_address) - first;
						SetDataInfoResult(data_info, total_values.at(word_index),
							DataFlowType::READ_RETURN, operation_results.at(word_index));
					}
					return data_info_vec;
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			case DataType::DF:
				// 发送 % 01 # RD D 00660 00661 54 \r(回车)
				// 回复 % 01$RD9A99B94110  解释为浮点数 41B9 999A 即23.200000762939453
				// 最大只能读26个字，即13个浮点，数据块DDF660 - 785，读数超过26个字，返回值截断结尾为 & 即无效BCC码
				try
				{
					std::vector<float> total_values;
					std::vector<int> operation_results;
					total_values.reserve(128);
					operation_results.reserve(128);
					for (auto& divided_range : data_block_range)
					{
						req = ADDR + "#RDD";
						req += UInt2ASCIIWithFixedDigits(divided_range.first, 5);
						if (divided_range.first == divided_range.second)
						{
							req += UInt2ASCIIWithFixedDigits(divided_range.first + 1, 5); // double value: address + 1
						}
						else
						{
							req += UInt2ASCIIWithFixedDigits(divided_range.second + 1, 5); // double value: address + 1
						}
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						// Calculate the reading float data count.
						int data_count = (divided_range.second == divided_range.first)
							? 1 : ((divided_range.second - divided_range.first) / 2 + 1);
						// Sync write
						//boost::asio::write(*_connection_manager, boost::asio::buffer(req));
						// Block write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::RDD write timeout or error." << std::endl;
#endif // DEBUG
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0.0f);
							continue;
						}
						// Block read
					    std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_READ_FLOAT_HEAD.size() ||
								reply.substr(0, REPLY_READ_FLOAT_HEAD.size()).compare(REPLY_READ_FLOAT_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD reply's head is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0.0f);
								continue;
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD reply's BCC checking is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0.0f);
								continue;
							}
							std::string data_str =
								reply.substr(REPLY_READ_FLOAT_HEAD.size(), reply.size() - REPLY_READ_FLOAT_HEAD.size() - 2/*BCC*/);
							if ((data_str.size() / 8/*double*/) != data_count)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::RDD reply's data count is error." << std::endl;
#endif // DEBUG
								operation_results.insert(operation_results.end(), data_count, EBADMSG);
								total_values.insert(total_values.end(), data_count, 0.0f);
								continue;
							}
							operation_results.insert(operation_results.end(), data_count, 0/*S_OK*/);
							std::vector<float> values = BCDStr2Float(data_str);
							total_values.insert(total_values.end(), values.begin(), values.end());
						}
						else
						{
#ifdef _DEBUG
							std::cerr << "fpplc::RDD reply timeout or error." << std::endl;
#endif // DEBUG
							operation_results.insert(operation_results.end(), data_count, ETIMEDOUT);
							total_values.insert(total_values.end(), data_count, 0.0f);
						}
					}
					for (auto& data_info : *data_info_vec)
					{
						std::size_t dword_index = (data_info.register_address - first) / 2;
						SetDataInfoResult(data_info, total_values.at(dword_index), 
							DataFlowType::READ_RETURN, operation_results.at(dword_index));
					}
					return data_info_vec;
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			default:
				throw std::invalid_argument("Unsupported fpplc data type.");
			}		
		}
		else
		{
			for (auto& data_info : *data_info_vec)
			{
				SetDataInfoResult(data_info, 0.0f, DataFlowType::READ_RETURN, ENOTCONN); // Set float defaultly.
			}
		}
		return data_info_vec; // Mark invalid and return.
	}

	std::shared_ptr<std::vector<DataInfo>> FpDriverWorker::WriteData(
		std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		// Pre-requisition: one serial port supports ONE panasonic fpplc, 
		// so all data_info.address is the same.
		const std::string ADDR = std::string("%") + UInt2ASCIIWithFixedDigits(data_info_vec->cbegin()->address, 2);

		const std::string END_OF_CMD = "\r";
		const std::string REPLY_WRITE_FLOAT_HEAD = ADDR + "$WD";
		const std::string REPLY_WRITE_WORD_HEAD = ADDR + "$WD";
		const std::string REPLY_WRITE_REGISTER_HEAD = ADDR + "$WC";
		const std::size_t TYPE_INDEX_BT = 0;
		const std::size_t TYPE_INDEX_DB = 1;
		const std::size_t TYPE_INDEX_DF = 2;
		const int START_LIMIT = 99999;
		const int END_LIMIT = -1;
		const int MAX_READ_WORD_COUNT = 26;

		std::vector<std::pair<int/*register address*/, int/*index*/>> register_data_index_vec;
		int i = 0;
		for (const auto& datainfo : *data_info_vec)
		{
			register_data_index_vec.push_back(std::make_pair(datainfo.register_address, i));
			i++;
		}
		std::sort(register_data_index_vec.begin(), register_data_index_vec.end(),
			[](const std::pair<int, int>& e1, const std::pair<int, int>& e2)
			{
				return e1.first < e2.first; // Sort by DataInfo.registger_address.
			});
        // Float registers: 2,4,6,10,14...58,68,70 => (2,4,6),(10),(14...38),(40...58),(68,70) with MAX_READ_WORD_COUNT limit
		// Word: 1,2,3,5,14...58,68,69 => (1,2,3),(5),(14...39),(40...58),(68,69) with MAX_READ_WORD_COUNT limit
		std::size_t word_offset = (data_info_vec->at(0).data_type == DataType::DF) ? 2 : 1;
		std::vector<std::vector<std::pair<int, int>>> data_block_range;
		std::size_t start = 0;
		std::size_t end = 0;
		for (std::size_t i = 1; i < register_data_index_vec.size(); i++)
		{
			if (register_data_index_vec.at(i).first == (register_data_index_vec.at(end).first + word_offset) &&
				(register_data_index_vec.at(i).first - register_data_index_vec.at(start).first + word_offset) <= MAX_READ_WORD_COUNT)
			{
				end = i;
				continue;
			}
			else
			{
				auto first = register_data_index_vec.cbegin() + start;
				auto last = register_data_index_vec.cbegin() + end + 1;
				data_block_range.push_back(std::vector<std::pair<int, int>>(first, last));
				start = i;
				end = start;
			}
		}
		auto first = register_data_index_vec.cbegin() + start;
		auto last = register_data_index_vec.cbegin() + end + 1;
		data_block_range.push_back(std::vector<std::pair<int, int>>(first, last));

		if (_connection_manager->is_open())
		{
			std::string req;
			switch (data_info_vec->at(0).data_type)
			{
			case DataType::BT:
				// Single contact
				// Send %01# WCS R 0108 1 2A \r
				// Reply %01$WC 14 \r 14 is BCC checking
				try
				{
					for (std::size_t i = 0; i < data_info_vec->size(); i++)
					{
						req = ADDR + "#WCSR";
						int word_index = data_info_vec->at(i).register_address / 16;
						unsigned short bit_index = data_info_vec->at(i).register_address % 16;
						req += UInt2ASCIIWithFixedDigits(word_index, 3);
						req += UInt2SingleBCDChar(bit_index);
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						// Blocking write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::WCSR write timeout or error." << std::endl;
#endif // DEBUG
							SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, ETIMEDOUT);
							continue;
						}
						// Blocking read
						std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_WRITE_REGISTER_HEAD.size() ||
								reply.substr(0, REPLY_WRITE_REGISTER_HEAD.size()).compare(REPLY_WRITE_REGISTER_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::WCSR reply's head is error." << std::endl;
#endif // DEBUG
								SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, EBADMSG);
								continue;
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::WCSR reply's BCC checking is error." << std::endl;
#endif // DEBUG
								SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, EBADMSG);
								continue;
							}
							SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, 0/*S_OK*/);
						}
						else
						{
							SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, ETIMEDOUT);
						}
					}
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			case DataType::WB:
				// Send %01# WDD 00585 00588 0000 0000 0000 0000 5D\r
				// Reply %01$WD13  13 is BCC checking.
				try
				{
					std::vector<float> total_values;
					for (auto& divided_range : data_block_range)
					{
						req = ADDR + "#WDD";
						req += UInt2ASCIIWithFixedDigits(divided_range.cbegin()->first, 5);
						req += UInt2ASCIIWithFixedDigits(divided_range.crbegin()->first, 5);
						std::vector<uint16_t> data_vec;
						for (const auto& item : divided_range)
						{
							data_vec.push_back(static_cast<uint16_t>(data_info_vec->at(item.second).int_value));
						}
						req += Word2BCDStr(data_vec);
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						// Blocking write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::WDD word write timeout or error." << std::endl;
#endif // DEBUG
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, ETIMEDOUT);
							}
							continue;
						}
						// Blocking read
						std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_WRITE_WORD_HEAD.size() ||
								reply.substr(0, REPLY_WRITE_WORD_HEAD.size()).compare(REPLY_WRITE_WORD_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::WDD word reply's head is error." << std::endl;
#endif // DEBUG
								for (const auto& item : divided_range)
								{
									SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, EBADMSG);
								}
								continue;
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::WDD word reply's BCC checking is error." << std::endl;
#endif // DEBUG
								for (const auto& item : divided_range)
								{
									SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, EBADMSG);
								}
								continue;
							}
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, 0/*S_OK*/);
							}
						}
						else
						{
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, ETIMEDOUT);
							}
						}
					}
					return data_info_vec;
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			case DataType::DF:
				// Send %01# WDD 00700 00703 0000 0000 0000 0000 53\r
				// Reply %01$WD13  13 is BCC checking.
				try
				{
					std::vector<float> total_values;
					for (auto& divided_range : data_block_range)
					{
						req = ADDR + "#WDD";
						req += UInt2ASCIIWithFixedDigits(divided_range.cbegin()->first, 5);
						req += UInt2ASCIIWithFixedDigits(divided_range.crbegin()->first + 1, 5);
						std::vector<float> data_vec;
						for (const auto& item : divided_range)
						{
							data_vec.push_back(static_cast<float>(data_info_vec->at(item.second).float_value));
						}
						req += Float2BCDStr(data_vec);
						req += BCCStr2BCDStr(req);
						req += END_OF_CMD;
						// Blocking write
						BlockingWriter writer(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						bool ok = writer.Write(req);
						if (!ok)
						{
#ifdef _DEBUG
							std::cerr << "fpplc::WDD write timeout or error." << std::endl;
#endif // DEBUG
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, ETIMEDOUT);
							}
							continue;
						}
						// Blocking read
						std::string reply;
						BlockingReader reader(_connection_manager, *_io_ctx, _connection_details.response_timeout_msec);
						ok = reader.ReadUntil(END_OF_CMD, reply);
						if (ok)
						{
							// Remove the tail "END_OF_CMD".
							reply.erase(reply.end() - END_OF_CMD.size());
							std::cout << reply << std::endl;
							if (reply.size() <= REPLY_WRITE_FLOAT_HEAD.size() ||
								reply.substr(0, REPLY_WRITE_FLOAT_HEAD.size()).compare(REPLY_WRITE_FLOAT_HEAD) != 0)
							{
#ifdef _DEBUG
								std::cerr << "fpplc::WDD reply's head is error." << std::endl;
#endif // DEBUG
								for (const auto& item : divided_range)
								{
									SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, EBADMSG);
								}
								continue;			
							}
							// BCC check
							std::string tail = reply.substr(reply.size() - 2, 2);
							std::string bcc = BCCStr2BCDStr(reply.substr(0, reply.size() - 2/*BCC*/));
							if (tail.compare(bcc) != 0)
							{
#ifdef _DEBUG

								std::cerr << "fpplc::WDD reply's BCC checking is error." << std::endl;
#endif // DEBUG
								for (const auto& item : divided_range)
								{
									SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, EBADMSG);
								}
								continue;
							}
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, 0/*S_OK*/);
							}
						}
						else
						{
							for (const auto& item : divided_range)
							{
								SetDataInfoResult(data_info_vec->at(item.second), DataFlowType::WRITE_RETURN, ETIMEDOUT);
							}
						}
					}
					return data_info_vec;
				}
				catch (boost::system::system_error& e)
				{
					std::string msg = "fpplc: port r/w exception error.";
					msg += e.what();
					throw std::runtime_error(msg);
				}
				break;
			default:
				throw std::invalid_argument("Unsupported fpplc data type.");
			}
		}
		else
		{
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				SetDataInfoResult(data_info_vec->at(i), DataFlowType::WRITE_RETURN, ENOTCONN);
			}
		}
		return data_info_vec;
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
		std::vector<char> bcd_codes {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
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
		};
		std::string str{ bcd_codes.at((bcc >> 4) & 0x0F), bcd_codes.at(bcc & 0x0F) };
		return str;
	}

	std::string FpDriverWorker::UInt2SingleBCDChar(unsigned int value)
	{
		// 0xF -> "F"
		std::vector<char> char_num{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		std::string str{ char_num.at(value & 0xF) };
		return str;
	}

	std::string FpDriverWorker::Float2BCDStr(const std::vector<float>& data_vec)
	{
		// 1.01f -> 0x3F8147AE -> "AE47813F"
		std::string str;
		std::vector<char> char_num {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		for (std::size_t i = 0; i < data_vec.size(); i++)
		{
			unsigned int value = *((unsigned int*)(&data_vec.at(i)));
			str += char_num.at((value >> 4) & 0xF);
			str += char_num.at((value >> 0) & 0xF);
			str += char_num.at((value >> 12) & 0xF);
			str += char_num.at((value >> 8) & 0xF);
			str += char_num.at((value >> 20) & 0xF);
			str += char_num.at((value >> 16) & 0xF);
			str += char_num.at((value >> 28) & 0xF);
			str += char_num.at((value >> 24) & 0xF);
		}
		return str;
	}

	std::string FpDriverWorker::Word2BCDStr(const std::vector<uint16_t>& data_vec)
	{
		// 0x0E0F -> "0F0E"
		std::string str;
		std::vector<char> char_num{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		for (std::size_t i = 0; i < data_vec.size(); i++)
		{
			str += char_num.at((data_vec.at(i) >> 4) & 0xF);
			str += char_num.at((data_vec.at(i) >> 0) & 0xF);
			str += char_num.at((data_vec.at(i) >> 12) & 0xF);
			str += char_num.at((data_vec.at(i) >> 8) & 0xF);
		}
		return str;
	}

	std::vector<float> FpDriverWorker::BCDStr2Float(const std::string& data_str)
	{
		std::vector<float> data_list;
		if (data_str.size() / 8 < 1 || data_str.size() / 8 > 9999)
		{
			return data_list;
		}
		for (std::size_t i = 0; i < data_str.size() / 8; i++)
		{
			unsigned int value = 0;
			value |= data_str.at(i * 8 + 6) > '9' ? (data_str.at(i * 8 + 6) - 'A' + 10) << 28 : (data_str.at(i * 8 + 6) - '0') << 28;
			value |= data_str.at(i * 8 + 7) > '9' ? (data_str.at(i * 8 + 7) - 'A' + 10) << 24 : (data_str.at(i * 8 + 7) - '0') << 24;
			value |= data_str.at(i * 8 + 4) > '9' ? (data_str.at(i * 8 + 4) - 'A' + 10) << 20 : (data_str.at(i * 8 + 4) - '0') << 20;
			value |= data_str.at(i * 8 + 5) > '9' ? (data_str.at(i * 8 + 5) - 'A' + 10) << 16 : (data_str.at(i * 8 + 5) - '0') << 16;
			value |= data_str.at(i * 8 + 2) > '9' ? (data_str.at(i * 8 + 2) - 'A' + 10) << 12 : (data_str.at(i * 8 + 2) - '0') << 12;
			value |= data_str.at(i * 8 + 3) > '9' ? (data_str.at(i * 8 + 3) - 'A' + 10) << 8 : (data_str.at(i * 8 + 3) - '0') << 8;
			value |= data_str.at(i * 8 + 0) > '9' ? (data_str.at(i * 8 + 0) - 'A' + 10) << 4 : (data_str.at(i * 8 + 0) - '0') << 4;
			value |= data_str.at(i * 8 + 1) > '9' ? (data_str.at(i * 8 + 1) - 'A' + 10) << 0 : (data_str.at(i * 8 + 1) - '0') << 0;
			float f = *((float*)&value);
			data_list.push_back(static_cast<float>(f));
		}
		return data_list;
	}

	std::vector<unsigned short> FpDriverWorker::BCDStr2Word(const std::string& data_str)
	{
		std::vector<unsigned short> data_list;
		if (data_str.size() / 4 < 1 || data_str.size() / 4 > 999)
		{
			return data_list;
		}
		for (std::size_t i = 0; i < data_str.size() / 4; i++)
		{
			unsigned short value = 0;
			value |= data_str.at(i * 4 + 2) > '9' ? (data_str.at(i * 4 + 2) - 'A' + 10) << 12 : (data_str.at(i * 4 + 2) - '0') << 12;
			value |= data_str.at(i * 4 + 3) > '9' ? (data_str.at(i * 4 + 3) - 'A' + 10) << 8 : (data_str.at(i * 4 + 3) - '0') << 8;
			value |= data_str.at(i * 4 + 0) > '9' ? (data_str.at(i * 4 + 0) - 'A' + 10) << 4 : (data_str.at(i * 4 + 0) - '0') << 4;
			value |= data_str.at(i * 4 + 1) > '9' ? (data_str.at(i * 4 + 1) - 'A' + 10) << 0 : (data_str.at(i * 4 + 1) - '0') << 0;
			data_list.push_back(value);
		}
		return data_list;
	}

	// Set DataInfo result fvalue
	void FpDriverWorker::SetDataInfoResult(DataInfo& data_info, float fvalue, DataFlowType data_flow_type, int result)
	{
		data_info.result = result;
		data_info.data_flow_type = data_flow_type;
		data_info.float_value = fvalue;
		data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
	}

	// Set DataInfo result bool value
	void FpDriverWorker::SetDataInfoResult(DataInfo& data_info, uint8_t bvalue, DataFlowType data_flow_type, int result)
	{
		data_info.result = result;
		data_info.data_flow_type = data_flow_type;
		data_info.byte_value = bvalue;
		data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
	}

	// Set DataInfo result ushort value
	void FpDriverWorker::SetDataInfoResult(DataInfo& data_info, uint16_t svalue, DataFlowType data_flow_type, int result)
	{
		data_info.result = result;
		data_info.data_flow_type = data_flow_type;
		data_info.int_value = svalue;
		data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
	}

	// Set DataInfo result without value
	void FpDriverWorker::SetDataInfoResult(DataInfo& data_info,  DataFlowType data_flow_type, int result)
	{
		data_info.result = result;
		data_info.data_flow_type = data_flow_type;
		data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
	}

	void FpDriverWorker::Test()
	{
		// test code 2,4,6,10,14...58,68
		auto vec = std::make_shared<std::vector<DataInfo>>();
		DataInfo a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16,
			a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29;
		a1.register_address = 714;
		a2.register_address = 716;
		a3.register_address = 700;
		a4.register_address = 702;
		a5.register_address = 704;
		a6.register_address = 706;
		a7.register_address = 708;
		//a8.register_address = 20;
		//a9.register_address = 22;
		//a10.register_address = 24;
		//a11.register_address = 26;
		//a12.register_address = 28;
		//a13.register_address = 30;
		//a14.register_address = 32;
		//a15.register_address = 34;
		//a16.register_address = 36;
		//a17.register_address = 38;
		//a18.register_address = 40;
		//a19.register_address = 42;
		//a20.register_address = 44;
		//a21.register_address = 46;
		//a22.register_address = 48;
		//a23.register_address = 50;
		//a24.register_address = 52;
		//a25.register_address = 54;
		//a26.register_address = 56;
		//a27.register_address = 58;
		//a28.register_address = 2;
		//a29.register_address = 70;

		vec->push_back(a1);
		vec->push_back(a2);
		vec->push_back(a3);
		vec->push_back(a4);
		vec->push_back(a5);
		vec->push_back(a6);
		vec->push_back(a7);
		//vec->push_back(a8);
		//vec->push_back(a9);
		//vec->push_back(a10);
		//vec->push_back(a11);
		//vec->push_back(a12);
		//vec->push_back(a13);
		//vec->push_back(a14);
		//vec->push_back(a15);
		//vec->push_back(a16);
		//vec->push_back(a17);
		//vec->push_back(a18);
		//vec->push_back(a19);
		//vec->push_back(a20);
		//vec->push_back(a21);
		//vec->push_back(a22);
		//vec->push_back(a23);
		//vec->push_back(a24);
		//vec->push_back(a25);
		//vec->push_back(a26);
		//vec->push_back(a27);
		//vec->push_back(a28);
		//vec->push_back(a29);
		for (auto& item : *vec)
		{
			item.data_type = DataType::DF;
			item.read_write_priviledge = ReadWritePrivilege::READ_WRITE;
		}
		WriteData(vec);
		
		// Word
		auto vec_word = std::make_shared<std::vector<DataInfo>>();
		DataInfo w1, w2, w3, w4;
		w1.register_address = 588;
		w2.register_address = 586;
		w3.register_address = 587;
		w4.register_address = 585;
		for (auto& item : *vec_word)
		{
			item.data_type = DataType::WB;
			item.read_write_priviledge = ReadWritePrivilege::READ_WRITE;
		}
		vec_word->push_back(w1);
		vec_word->push_back(w2);
		vec_word->push_back(w3);
		vec_word->push_back(w4);
		WriteData(vec_word);
	}
}

