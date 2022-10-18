// S7 driver worker.
// <1>
// Write bit of bits: https://sourceforge.net/p/snap7/discussion/general/thread/b4c7b4f4/
// You can use WriteArea passing S7WlBit as wordlength.
// The CP(communicator processor) changes only the desired bit.
// To write more bit at time you can use WriteMultiVars.
// <2>
// The SmartConnect feature relies on this principle to avoid the TCP connection timeout when a PLC is off or the network cable is unwired.
// Unlike the TCP connection timeout, The ping time is fixedand we can decide how much it should be.
// <3>
// Particularly to access a DB in S71500 some additional setting plc-side are needed.
// 1.Only global DBs can be accessed.
// 2.The optimized block access must be turned off.
// 3.The access level must be ¡°full¡± and the ¡°connection mechanism¡± must allow GET / PUT.

#include "pch.h"
#include <iostream>
#include <vector>
#include "S7DriverWorker.h"
#include <cassert>

namespace goiot
{
	void S7DriverWorker::Test()
	{
		// PLC status test
		S7CPUStatus status = PLCStatus();
		// DBRead test
		std::vector<uint8_t> data_vec(1000);
		int read_db_ret = connection_manager_->DBRead(1, 0, 22, &data_vec.at(0));
		//int size = 16;
		//read_db_ret = connection_manager_->DBGet(1, &data_vec.at(0), &size); // errCliFunNotAvailable
		// DBWrite test
		data_vec.at(0) = 0xfe;
		int write_db_ret = connection_manager_->DBWrite(1, 12, 1, &data_vec.at(0));
		// WriteArea test
		uint8_t status_byte = 0;
		int write_bit_ret = connection_manager_->WriteArea(S7AreaDB, 1, 12 * 8 + 1, 1, S7WLBit, &status_byte);
		// WriteMultiVars test
		std::shared_ptr<std::vector<DataInfo>> data_info_vec(new std::vector<DataInfo>(4));
		data_info_vec->at(0).id = "id1";
		data_info_vec->at(0).address = 1;
		data_info_vec->at(0).register_address = 0;
		data_info_vec->at(0).data_type = DataType::DB;
		data_info_vec->at(0).int_value = 88886666;
		data_info_vec->at(1).id = "id2";
		data_info_vec->at(1).address = 1;
		data_info_vec->at(1).register_address = 16 * 8 + 1;
		data_info_vec->at(1).data_type = DataType::BT;
		data_info_vec->at(1).byte_value = 0;
		data_info_vec->at(2).id = "id3";
		data_info_vec->at(2).address = 1;
		data_info_vec->at(2).register_address = 18;
		data_info_vec->at(2).data_type = DataType::DF;
		data_info_vec->at(2).float_value = -2000.08;
		data_info_vec->at(3).id = "id4";
		data_info_vec->at(3).address = 1;
		data_info_vec->at(3).register_address = 22;
		data_info_vec->at(3).data_type = DataType::WB;
		data_info_vec->at(3).int_value = 19990;

		auto rp_w_vec = WriteData(data_info_vec);
		data_info_vec->at(0).int_value = 0;
		data_info_vec->at(1).byte_value = 1;
		data_info_vec->at(2).float_value = 0.0;
		data_info_vec->at(3).int_value = 0;
		auto rp_r_vec = ReadData(data_info_vec);
	}

	int S7DriverWorker::OpenConnection()
	{
		connection_manager_.reset(new TS7Client,
			[](TS7Client* p) { p->Disconnect(); std::cout << "free S7 ptr."; });
		int res = connection_manager_->ConnectTo(connection_details_.port.c_str(),
			connection_details_.rack, connection_details_.slot);
		if (res != 0)
		{
			std::cerr << "s7_driver Connection failed: " << CliErrorText(res) << std::endl;
			return ENOTCONN;
		}
		else
		{
			std::cout << "s7_driver Connected to: " << connection_details_.port << " Rack=" <<
				connection_details_.rack << ", Slot=" << connection_details_.slot << std::endl;
		}
		return res;
	}

	void S7DriverWorker::CloseConnection()
	{
		connection_manager_.reset();
#ifdef _DEBUG
		std::cout << "s7_driver worker closed connection.";
#endif // _DEBUG
	}

	// Spawns working threads.
	void S7DriverWorker::Start()
	{
		refresh_ = true;
		threads_.emplace_back(&S7DriverWorker::Refresh, this);
		threads_.emplace_back(&S7DriverWorker::Request_Dispatch, this);
		threads_.emplace_back(&S7DriverWorker::Response_Dispatch, this);
	}

	// Stops and waits the working threads completed.
	void S7DriverWorker::Stop()
	{
		refresh_ = false;
		in_queue_.Close();
		out_queue_.Close();
		for (auto& entry : threads_)
		{
			entry.join();
		}
	}

	// Refreshs the data reading or writing requests into in_queue.
	void S7DriverWorker::Refresh()
	{
		while (refresh_)
		{
			try
			{
				std::vector<DataInfo> data_info_vec;
				for (auto data_info : data_map_)
				{
					data_info_vec.emplace_back(data_info.second.id,
						data_info.second.name, data_info.second.address, data_info.second.register_address,
						data_info.second.read_write_priviledge, DataFlowType::REFRESH, data_info.second.data_type,
						data_info.second.data_zone, data_info.second.float_decode, data_info.second.dword_decode, data_info.second.byte_value, data_info.second.int_value, data_info.second.float_value,
						data_info.second.char_value, std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0);
				}
				if (data_info_vec.size() > 0)
				{
					in_queue_.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec), 
						true, std::chrono::milliseconds(1000));
				}
			}
			catch (const QFull&)
			{
				std::cerr << "s7_driver:In-queue is full" << std::endl;
			}
			catch (const std::exception & e) {
				std::cerr << "s7_driver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "s7_driver:EXCEPTION (unknown)" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(connection_details_.refresh_interval_msec));
		}
	}

	// Dispatch worker deals with the in_queue request, which may read/write message to hardware.
	// The return data are put into the out_queue.
	void S7DriverWorker::Request_Dispatch()
	{
		while (true)
		{
			try
			{
				std::shared_ptr<std::vector<DataInfo>> data_info_vec;
				in_queue_.Get(data_info_vec);
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
					rp_data_info_vec = ReadBatchData(data_info_vec); // Modify data_info_vec directly, may be improve.
				}
				else if (data_info_vec->at(0).data_flow_type == DataFlowType::ASYNC_WRITE)
				{
					rp_data_info_vec = WriteData(data_info_vec);
				}
				else
				{
					throw std::invalid_argument("Unsupported data flow");
				}
				out_queue_.Put(rp_data_info_vec, true, std::chrono::milliseconds(1000));
			}
			catch (const QFull&)
			{
				std::cerr << "s7_driver:Out-queue is full" << std::endl;
			}
			catch (const std::exception & e) {
				std::cerr << "s7_driver:EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "s7_driver:EXCEPTION (unknown)" << std::endl;
			}
		}
	}

	// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
	void S7DriverWorker::Response_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			out_queue_.Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			try
			{
				driver_manager_reponse_queue_->PutNoWait(data_info_vec);
			}
			catch (const QFull&)
			{
				assert(false);
				std::cout << "s7_driver:driver_manager_response_queue_ is full." << std::endl;
			}
		}
	}

	// Puts asynchronous read request to the in_queue.
	void S7DriverWorker::AsyncRead(const std::vector<DataInfo>& data_info_vec, int trans_id)
	{
		if (data_info_vec.size() > 0)
		{
			assert(data_info_vec.at(0).data_flow_type == DataFlowType::ASYNC_READ);
			in_queue_.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec));
		}
	}

	// Puts asynchronous write request to the out_queue.
	int S7DriverWorker::AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id)
	{
		if (data_info_vec.size() > 0)
		{
			assert(data_info_vec.at(0).data_flow_type == DataFlowType::ASYNC_WRITE);
			try
			{
				in_queue_.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec),
					true, std::chrono::milliseconds(200)); // prevent blocking DataService poll dispatch()
			}
			catch (const QFull&)
			{
#ifdef _DEBUG
				std::cout << "S7 in_queue_ is full." << std::endl;
#endif // _DEBUG
				return ETIMEDOUT;
			}
		}
		return 0;
	}

	//Read data from device.
	std::shared_ptr<DataInfo> S7DriverWorker::ReadData(const DataInfo& data_info)
	{
		std::shared_ptr<DataInfo> rd(std::make_shared<DataInfo>(data_info));
		rd->data_flow_type = DataFlowType::READ_RETURN; // default data_flow_type: read_return
		rd->result = ENODATA; // default result: no data

		if (Connected())
		{
			int bytes = GetNumberOfByte(data_info.data_type);
			std::vector<uint8_t> data_vec(bytes);
			int s7_len = 0;
			// Assign S7 Word Length
			switch (data_info.data_type)
			{
			case DataType::BT:
				s7_len = S7WLBit;
				break;
			case DataType::BB:
				s7_len = S7WLByte;
				break;
			case DataType::WB:
			case DataType::WUB:
				s7_len = S7WLWord;
				break;
			case DataType::DB:
			case DataType::DUB:
				s7_len = S7WLDWord;
				break;
			case DataType::DF:
				s7_len = S7WLReal;
				break;
			default:
				throw std::invalid_argument("Unsupported data type");		
			}
			int read_db_ret = connection_manager_->ReadArea(S7AreaDB, data_info.address, 
				data_info.register_address, 1/* Amount */, s7_len, &data_vec.at(0));
			if (read_db_ret != 0)
			{
				rd->result = ENODATA;
			}
			else
			{
				rd->result = 0;
				switch (data_info.data_type)
				{
				case DataType::BT:
				case DataType::BB:
					rd->byte_value = data_vec.at(0); // Store boolean into uint8_t
					break;
				case DataType::WB:
				case DataType::WUB:
					rd->int_value = GetInt16(data_vec, 0);
					break;
				case DataType::DB:
				case DataType::DUB:
					rd->int_value = GetInt(data_vec, 0);
					break;
				case DataType::DF:
					rd->float_value = GetFloat(data_vec, 0);
					break;
				default:
					throw std::invalid_argument("Unsupported data type");
				}
			}
		}
		else
		{
			Connect();
			rd->result = ENOTCONN;
		}
		rd->timestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
		return rd;
	}

	// Read multiple data.
	std::shared_ptr<std::vector<DataInfo>> S7DriverWorker::ReadData(std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (data_info_vec == nullptr)
		{
			throw std::invalid_argument("data_info_vec is null.");
		}

		if (Connected())
		{
			int byte_sum = 0;
			std::vector<int> data_start_pos_vec(data_info_vec->size()); // every data start position in data_vec in byte
			std::vector<int> s7_len_vec(data_info_vec->size());
			std::vector<TS7DataItem> data_item_vec(data_info_vec->size());
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_start_pos_vec.at(i) = byte_sum;
				switch (data_info_vec->at(i).data_type)
				{
				case DataType::BT:
					s7_len_vec.at(i) = S7WLBit;
					byte_sum += 1;
					break;
				case DataType::BB:
					s7_len_vec.at(i) = S7WLByte;
					byte_sum += 1;
					break;
				case DataType::WB:
				case DataType::WUB:
					s7_len_vec.at(i) = S7WLWord;
					byte_sum += 2;
					break;
				case DataType::DB:
				case DataType::DUB:
					s7_len_vec.at(i) = S7WLDWord;
					byte_sum += 4;
					break;
				case DataType::DF:
					s7_len_vec.at(i) = S7WLReal;
					byte_sum += 4;
					break;
				default:
					throw std::invalid_argument("Unsupported data type");
				}
			}
			// Assign read cache
			std::vector<uint8_t> data_vec(byte_sum); 
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_item_vec.at(i).Amount = 1;
				data_item_vec.at(i).Area = S7AreaDB;
				data_item_vec.at(i).DBNumber = data_info_vec->at(i).address;
				data_item_vec.at(i).pdata = &data_vec.at(data_start_pos_vec.at(i));
				data_item_vec.at(i).Result = -1;
				data_item_vec.at(i).Start = data_info_vec->at(i).register_address;
				data_item_vec.at(i).WordLen = s7_len_vec.at(i);
			}
			int read_ret = connection_manager_->ReadMultiVars(&data_item_vec.at(0), data_item_vec.size());
			if (read_ret != 0)
			{
				for (std::size_t i = 0; i < data_info_vec->size(); i++)
				{
					data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
					data_info_vec->at(i).result = ENODATA;
					data_info_vec->at(i).timestamp =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
				}
			}
			else
			{
				for (std::size_t i = 0; i < data_info_vec->size(); i++)
				{
					data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
					data_info_vec->at(i).result = (data_item_vec.at(i).Result == 0) ? 0 : ENODATA;
					// parse return data
					switch (data_info_vec->at(i).data_type)
					{
					case DataType::BT:
					case DataType::BB:
						data_info_vec->at(i).byte_value = data_vec.at(data_start_pos_vec.at(i)); // Store boolean into uint8_t
						break;
					case DataType::WB:
					case DataType::WUB:
						data_info_vec->at(i).int_value = GetInt16(data_vec, data_start_pos_vec.at(i));
						break;
					case DataType::DB:
					case DataType::DUB:
						data_info_vec->at(i).int_value = GetInt(data_vec, data_start_pos_vec.at(i));
						break;
					case DataType::DF:
						data_info_vec->at(i).float_value = GetFloat(data_vec, data_start_pos_vec.at(i));
						break;
					default:
						throw std::invalid_argument("Unsupported data type");
					}
					data_info_vec->at(i).timestamp =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
				}
			}
		}
		else
		{
			Connect();
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
				data_info_vec->at(i).result = ENOTCONN;
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		return data_info_vec;
	}

	// Read DB block and parse multiple data.
	std::shared_ptr<std::vector<DataInfo>> S7DriverWorker::ReadBatchData(std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (Connected())
		{
			std::map<int/* db id */, std::vector<byte>> data_map;
			for (auto& entry : db_mapping_)
			{
				data_map.insert({ entry.first/* DB id */, std::vector<uint8_t>(entry.second.second - entry.second.first) });
			}
			std::map<int/* db id */, int/* error_code */> ret_vec;
			for (auto& entry : db_mapping_)
			{
				int read_db_ret = connection_manager_->DBRead(entry.first, entry.second.first/* start */,
					entry.second.second - entry.second.first/* size */, &data_map[entry.first].at(0));
				ret_vec[entry.first] = read_db_ret;
			}
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
				if (ret_vec[data_info_vec->at(i).address/* db id */] != 0)
				{
					data_info_vec->at(i).result = ENODATA;
				}
				else
				{
					data_info_vec->at(i).result = 0;
					int address = data_info_vec->at(i).address;
					int register_address = data_info_vec->at(i).register_address;
					std::size_t pos = register_address/* absolute offset */ - db_mapping_[address].first/* db read start */;
					switch (data_info_vec->at(i).data_type)
					{
					case DataType::BT:
						pos = register_address / 8/* absolute offset */ - db_mapping_[address].first/* db read start */;
						data_info_vec->at(i).byte_value = (data_map[address].at(pos) & (1 << (register_address % 8))) ? 1 : 0; // Store boolean into uint8_t
						break;
					case DataType::BB:
						data_info_vec->at(i).byte_value = data_map[address].at(pos);
						break;
					case DataType::WB:
					case DataType::WUB:
						data_info_vec->at(i).int_value = GetInt16(data_map[address], pos);
						break;
					case DataType::DB:
					case DataType::DUB:
						data_info_vec->at(i).int_value = GetInt(data_map[address], pos);
						break;
					case DataType::DF:
						data_info_vec->at(i).float_value = GetFloat(data_map[address], pos);
						break;
					default:
						throw std::invalid_argument("Unsupported data type");
					}
				}
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		else
		{
			Connect();
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::READ_RETURN;
				data_info_vec->at(i).result = ENOTCONN;
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		return data_info_vec;
	}

	int S7DriverWorker::WriteData(const DataInfo& data_info)
	{
		if (Connected())
		{
			int bytes = GetNumberOfByte(data_info.data_type);
			std::vector<uint8_t> data_vec(bytes);
			int s7_len = 0;
			float f = 0.0;
			uint8_t* byte_addr = 0;
			// Assign S7 Word Length
			switch (data_info.data_type)
			{
			case DataType::BT:
				s7_len = S7WLBit;
				data_vec.at(0) = data_info.byte_value;
				break;
			case DataType::BB:
				s7_len = S7WLByte;
				data_vec.at(0) = data_info.byte_value;
				break;
			case DataType::WB:
			case DataType::WUB:
				s7_len = S7WLWord;
				data_vec.at(0) = (data_info.int_value & 0xff00) >> 8;
				data_vec.at(1) = data_info.int_value & 0x00ff;
				break;
			case DataType::DB:
			case DataType::DUB:
				s7_len = S7WLDWord;
				data_vec.at(0) = data_info.int_value >> 24;
				data_vec.at(1) = (data_info.int_value >> 16) & 0x00ff;
				data_vec.at(2) = (data_info.int_value & 0xff00) >> 8;
				data_vec.at(3) = data_info.int_value & 0x00ff;
				break;
			case DataType::DF:
				s7_len = S7WLReal;
				f = static_cast<float>(data_info.float_value);
				byte_addr = (uint8_t*)&f;
				data_vec.at(0) = *(byte_addr + 3);
				data_vec.at(1) = *(byte_addr + 2);
				data_vec.at(2) = *(byte_addr + 1);
				data_vec.at(3) = *byte_addr;
				break;
			default:
				throw std::invalid_argument("Unsupported data type");
			}
			int write_ret = connection_manager_->WriteArea(S7AreaDB, data_info.address,
				data_info.register_address, 1/* Amount */, s7_len, &data_vec.at(0));
			if (write_ret != 0)
			{
				return ENODATA;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return ENOTCONN;
		}
	}

	std::shared_ptr<std::vector<DataInfo>> S7DriverWorker::WriteData(std::shared_ptr<std::vector<DataInfo>> data_info_vec)
	{
		if (data_info_vec == nullptr)
		{
			throw std::invalid_argument("data_info_vec is null.");
		}

		if (Connected())
		{
			int byte_sum = 0;
			std::vector<uint8_t> data_vec;
			data_vec.reserve(1024);
			std::vector<int> data_start_pos_vec(data_info_vec->size()); // every data start position in data_vec in byte
			std::vector<int> s7_len_vec(data_info_vec->size());
			std::vector<TS7DataItem> data_item_vec(data_info_vec->size());
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_start_pos_vec.at(i) = byte_sum;
				float f = 0.0;
				uint8_t* byte_addr = 0;
				switch (data_info_vec->at(i).data_type)
				{
				case DataType::BT:
					data_vec.push_back(data_info_vec->at(i).byte_value);
					s7_len_vec.at(i) = S7WLBit;
					byte_sum += 1;
					break;
				case DataType::BB:
					data_vec.push_back(data_info_vec->at(i).byte_value);
					s7_len_vec.at(i) = S7WLByte;
					byte_sum += 1;
					break;
				case DataType::WB:
				case DataType::WUB:
					data_vec.push_back((data_info_vec->at(i).int_value & 0xff00) >> 8);
					data_vec.push_back(data_info_vec->at(i).int_value & 0x00ff);
					s7_len_vec.at(i) = S7WLWord;
					byte_sum += 2;
					break;
				case DataType::DB:
				case DataType::DUB:
					data_vec.push_back(data_info_vec->at(i).int_value >> 24);
					data_vec.push_back((data_info_vec->at(i).int_value >> 16) & 0x00ff);
					data_vec.push_back((data_info_vec->at(i).int_value & 0xff00) >> 8);
					data_vec.push_back(data_info_vec->at(i).int_value & 0x00ff);
					s7_len_vec.at(i) = S7WLDWord;
					byte_sum += 4;
					break;
				case DataType::DF:
					f = static_cast<float>(data_info_vec->at(i).float_value);
					byte_addr = (uint8_t*)&f;
					data_vec.push_back(*(byte_addr + 3));
					data_vec.push_back(*(byte_addr + 2));
					data_vec.push_back(*(byte_addr + 1));
					data_vec.push_back(*byte_addr);
					s7_len_vec.at(i) = S7WLReal;
					byte_sum += 4;
					break;
				default:
					throw std::invalid_argument("Unsupported data type");
				}
			}
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_item_vec.at(i).Amount = 1;
				data_item_vec.at(i).Area = S7AreaDB;
				data_item_vec.at(i).DBNumber = data_info_vec->at(i).address;
				data_item_vec.at(i).pdata = &data_vec.at(data_start_pos_vec.at(i));
				data_item_vec.at(i).Result = -1;
				data_item_vec.at(i).Start = data_info_vec->at(i).register_address;
				data_item_vec.at(i).WordLen = s7_len_vec.at(i);
			}
			int write_ret = connection_manager_->WriteMultiVars(&data_item_vec.at(0), data_item_vec.size());
			if (write_ret != 0)
			{
				for (std::size_t i = 0; i < data_info_vec->size(); i++)
				{
					data_info_vec->at(i).data_flow_type = DataFlowType::WRITE_RETURN;
					data_info_vec->at(i).result = ENODATA;
					data_info_vec->at(i).timestamp = 
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
				}
			}
			else
			{
				for (std::size_t i = 0; i < data_info_vec->size(); i++)
				{
					data_info_vec->at(i).data_flow_type = DataFlowType::WRITE_RETURN;
					data_info_vec->at(i).result = (data_item_vec.at(i).Result == 0) ? 0 : ENODATA;
					data_info_vec->at(i).timestamp =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
				}
			}
		}
		else
		{
			Connect();
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				data_info_vec->at(i).data_flow_type = DataFlowType::WRITE_RETURN;
				data_info_vec->at(i).result = ENOTCONN;
				data_info_vec->at(i).timestamp =
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
			}
		}
		return data_info_vec;
	}

	int S7DriverWorker::GetNumberOfByte(DataType datatype)
	{
		switch (datatype)
		{
		case DataType::BT:
		case DataType::BB:
			return 1;
		case DataType::WB:
		case DataType::WUB:
			return 2;
		case DataType::DB:
		case DataType::DF:
		case DataType::DUB:
			return 4;
		default:
			throw std::invalid_argument("Unsupported data type.");
		}
	}

	// Get a float from 4 tytes
	float S7DriverWorker::GetFloat(const std::vector<byte>& src, std::size_t pos)
	{
		float f;
		uint32_t i = 
			(src.at(pos) << 24) + (src.at(pos + 1) << 16) + (src.at(pos + 2) << 8) + src.at(pos + 3);
		memcpy(&f, &i, sizeof(float));
		return f;
	}

	// Get an integer from 4 bytes
	int S7DriverWorker::GetInt(const std::vector<byte>& src, std::size_t pos)
	{
		return (src.at(pos) << 24) + (src.at(pos + 1) << 16) + (src.at(pos + 2) << 8) + src.at(pos + 3);
	}

	// Get an short integer from 4 bytes
	int16_t S7DriverWorker::GetInt16(const std::vector<byte> src, std::size_t pos)
	{
		return (src.at(pos) << 8) + src.at(pos + 1);
	}

	S7CPUStatus S7DriverWorker::PLCStatus()
	{
		int status = connection_manager_->PlcStatus();
		switch (status)
		{
		case S7CpuStatusRun: 
			printf("  RUN\n"); 
			return S7CPUStatus::Run;
		case S7CpuStatusStop: 
			printf("  STOP\n"); 
			return S7CPUStatus::Stop;
		default: 
			printf("  UNKNOWN\n"); 
			return S7CPUStatus::Unknown;
		}
	}

	// ListBlocks is not supported by S7-1500
	void S7DriverWorker::ListBlocks() 
	{
		TS7BlocksList list;
		int res = connection_manager_->ListBlocks(&list);
		if (res == 0)
		{
			printf("  OBCount  : %d\n", list.OBCount);
			printf("  FBCount  : %d\n", list.FBCount);
			printf("  FCCount  : %d\n", list.FCCount);
			printf("  SFBCount : %d\n", list.SFBCount);
			printf("  SFCCount : %d\n", list.SFCCount);
			printf("  DBCount  : %d\n", list.DBCount);
			printf("  SDBCount : %d\n", list.SDBCount);
		};
	}

	// Parse data block start and end offset for read optimizaiton.
	void S7DriverWorker::ParseDBOffset()
	{
		if (data_map_.size() == 0)
		{
			return;
		}
		int start = 0;  // 
		int end = 0; // not included
		for (auto& entry : data_map_)
		{
			switch (entry.second.data_type)
			{
			case DataType::BT:
				start = entry.second.register_address / 8;  // to byte
				end = start + 1;
				break;
			case DataType::BB:
				start = entry.second.register_address;  // to byte
				end = start + 1;
				break;
			case DataType::DB:
			case DataType::DF:
			case DataType::DUB:
				start = entry.second.register_address;  // in byte
				end = start + 4;
				break;
			case DataType::WB:
			case DataType::WUB:
				start = entry.second.register_address;  // in byte
				end = start + 2;
				break;
			default:
				throw std::invalid_argument("Unsupported S7 data type");
			}
			if (db_mapping_.find(entry.second.address) == db_mapping_.end())
			{
				db_mapping_.insert({entry.second.address, std::make_pair(512/* db_start_offset_ init value */, 0)});
			}
			if (start < db_mapping_[entry.second.address].first && start >= 0)
			{
				db_mapping_[entry.second.address].first = start;
			}
			if (end > db_mapping_[entry.second.address].second && end > 0)
			{
				db_mapping_[entry.second.address].second = end;
			}
		}
	}
}