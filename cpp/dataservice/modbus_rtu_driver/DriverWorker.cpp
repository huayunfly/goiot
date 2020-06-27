// DriverWorker implementation.
// @author: Yun Hua (huayunfly@126.com)
// @version: 0.1 2020.06.27
// 
// Modbus is working on master to multiple slaves mode.
// Sending command (master -> slave)
// 1byte slave address, 1 byte function code, 0~252 byte data, 2 byte CRC
// 00h: the broadcast address
//
// Write coil ON/OFF, ON: 1, OFF: 0
//
// 读取线圈（01h）
// 仅返回由线圈起始地址中设置的地址开始的线圈数对应的线圈信息。
// 关于数据字节数（N），用线圈数除以 8，没有余数时，直接返回商，有余数时，返回“商 + 1”。
// 有余数时，在最后的数据中指定的线圈数的范围外为「0」。
//
// 读取寄存器（03h）
//
// 写入线圈（05h）
// ON / OFF 指定地址的线圈。
// ON ：変更数据 高位 FFh、低位 00h
// OFF ：変更数据 高位 00h、低位 00h
//
// 写入寄存器（06h）
// 通信判断（08h）
//
// 复数线圈的写入（0Fh）
// 向起始地址中指定的线圈开始的指定数量的线圈中写入数据。
// 关于数据字节数（N），用线圈数除以 8，没有余数时，直接设定为商，有余数时，设定为“商 + 1”。
// 变更数据从开始地址中指定的线圈开始，分别用 1bit 数据（1 / 0）依次设定各线圈的 ON / OFF。
//
// 复数寄存器的写入（10h）

#include "pch.h"
#include <mutex>
#include <iostream>
#include "DriverWorker.h"

namespace goiot
{
	int DriverWorker::OpenConnection()
	{
		connection_manager_.reset(modbus_new_rtu(connection_details_.port.c_str(),
			connection_details_.baud, connection_details_.parity, connection_details_.data_bit, connection_details_.stop_bit),
			[](modbus_t* p) { modbus_close(p);  modbus_free(p); std::cout << "free modbus ptr."; }
		);
		modbus_set_debug(connection_manager_.get(), FALSE);

		// For test
		//const uint16_t UT_BITS_NB = 0x25;
		//uint8_t tab_value[UT_BITS_NB];
		//const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };  // 0xCD 1100 1101
		//modbus_set_bits_from_bytes(tab_value, 0, UT_BITS_NB, UT_BITS_TAB); // tab_value [ 1 0 1 1 0 0 1 1 ... ]
		//const float UT_REAL = 123456.00;
		//uint16_t* tab_rp_registers = (uint16_t*)malloc(2 * sizeof(uint16_t));
		//memset(tab_rp_registers, 0, 2 * sizeof(uint16_t));
		//modbus_set_float_abcd(UT_REAL, tab_rp_registers);

		uint32_t old_response_to_sec;
		uint32_t old_response_to_usec;
		modbus_get_response_timeout(connection_manager_.get(), &old_response_to_sec, &old_response_to_usec);
		int response_to_sec = connection_details_.response_to_msec / 1000;
		int response_to_micro_sec = connection_details_.response_to_msec % 1000;
		modbus_set_response_timeout(connection_manager_.get(), response_to_sec, response_to_micro_sec * 1000);
		if (modbus_connect(connection_manager_.get()) == -1) {
			std::cout << "Connection failed: " << modbus_strerror(errno) << std::endl;
			return ENOTCONN;
		}
		uint32_t new_response_to_sec;
		uint32_t new_response_to_usec;
		modbus_get_response_timeout(connection_manager_.get(), &new_response_to_sec, &new_response_to_usec);
		return 0;
	}

	void DriverWorker::CloseConnection()
	{
		connection_manager_.reset();
	}

	void DriverWorker::Start()
	{
		std::call_once(connection_init_flag_, &DriverWorker::OpenConnection, this);
		refresh_ = true;
		threads_.emplace_back(&DriverWorker::Refresh, this);
		threads_.emplace_back(&DriverWorker::Request_Dispatch, this);
		threads_.emplace_back(&DriverWorker::Response_Dispatch, this);
	}

	void DriverWorker::Stop()
	{
		refresh_ = false;
		in_queue_.Close();
		out_queue_.Close();
		for (auto& entry : threads_)
		{
			entry.join();
		}
	}

	void DriverWorker::Refresh()
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
						data_info.second.data_zone, data_info.second.float_decode, data_info.second.int_value, data_info.second.float_value,
						data_info.second.char_value, std::chrono::system_clock().now().time_since_epoch().count());
				}
				if (data_info_vec.size() > 0)
				{
					in_queue_.Put(std::make_shared<std::vector<DataInfo>>(data_info_vec));
				}
			}
			catch (const Full&)
			{
				std::cerr << "In-queue is full" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "EXCEPTION (unknown)" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	void DriverWorker::Request_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			in_queue_.Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			auto rp_data_info_vec = std::make_shared<std::vector<DataInfo>>();
			for (std::size_t i = 0; i < data_info_vec->size(); i++)
			{
				std::shared_ptr<DataInfo> data_info;
				switch (data_info_vec->at(i).data_flow_type)
				{
				case DataFlowType::REFRESH:
					data_info = ReadData(data_info_vec->at(i));
					// Copy data, may be improved.
					rp_data_info_vec->emplace_back(data_info->id,
						data_info->name, data_info->address, data_info->register_address,
						data_info->read_write_priviledge, DataFlowType::REFRESH, data_info->data_type,
						data_info->data_zone, data_info->float_decode, data_info->int_value, data_info->float_value,
						data_info->char_value, std::chrono::system_clock::now().time_since_epoch().count());
					break;
				default:
					break;
				}
			}
			out_queue_.Put(rp_data_info_vec);
		}
	}

	void DriverWorker::Response_Dispatch()
	{
		while (true)
		{
			std::shared_ptr<std::vector<DataInfo>> data_info_vec;
			out_queue_.Get(data_info_vec);
			if (data_info_vec == nullptr) // Improve for a robust SENTINEL
			{
				break; // Exit
			}
			driver_manager_reponse_queue_->PutNoWait(data_info_vec);
		}
	}

	// Read modbus device data
	std::shared_ptr<DataInfo> DriverWorker::ReadData(const DataInfo& data_info)
	{
		int rc;
		modbus_set_slave(connection_manager_.get(), data_info.address);
		int num_registers = GetNumberOfRegister(data_info.data_type);
		int num_bits = 1;
		std::shared_ptr<uint16_t> rp_registers;
		std::shared_ptr<uint8_t> rp_bits;
		std::shared_ptr<DataInfo> rd(std::make_shared<DataInfo>(data_info));
		rd->data_flow_type = DataFlowType::READ_RETURN; // Init to read_return
		rd->result = ENODATA; // Init to no_message_available
		switch (data_info.data_zone)
		{
		case DataZone::OUTPUT_RELAY:
			rp_bits.reset(new uint8_t[num_bits], std::default_delete<uint8_t[]>());
			memset(rp_bits.get(), 0, num_bits * sizeof(uint8_t));
			rc = modbus_read_bits(connection_manager_.get(), data_info.register_address,
				num_bits, rp_bits.get());
			if (rc != num_bits) 
			{
#ifdef _DEBUG
				std::cerr << "Read output bits " << std::hex << std::showbase << data_info.register_address << " failed." << std::endl;
#endif // DEBUG
			}
			AssignBitValue(rd, rp_bits);
			rd->result = 0;
			break;
		case DataZone::INPUT_RELAY:
			rp_bits.reset(new uint8_t[num_bits], std::default_delete<uint8_t[]>());
			memset(rp_bits.get(), 0, num_bits * sizeof(uint8_t));
			rc = modbus_read_input_bits(connection_manager_.get(), data_info.register_address,
				num_bits, rp_bits.get());
			if (rc != num_bits)
			{
#ifdef _DEBUG
				std::cerr << "Read input bits " << std::hex << std::showbase << data_info.register_address << " failed." << std::endl;
#endif // DEBUG
			}
			AssignBitValue(rd, rp_bits);
			rd->result = 0;
			break;
		case DataZone::OUTPUT_REGISTER:
			rp_registers.reset(new uint16_t[num_registers], std::default_delete<uint16_t[]>()); // Calls delete[] as deleter
			memset(rp_registers.get(), 0, num_registers * sizeof(uint16_t));
			rc = modbus_read_registers(connection_manager_.get(), data_info.register_address,
				num_registers, rp_registers.get());
			if (rc != num_registers) 
			{
#ifdef _DEBUG
				std::cerr << "Read output registers " << std::hex << std::showbase << data_info.register_address << " failed." << std::endl;
#endif // DEBUG
			}
			AssignRegisterValue(rd, rp_registers);
			rd->result = 0;
			break;
		case DataZone::INPUT_REGISTER:
			rp_registers.reset(new uint16_t[num_registers], std::default_delete<uint16_t[]>()); // Calls delete[] as deleter
			memset(rp_registers.get(), 0, num_registers * sizeof(uint16_t));
			rc = modbus_read_input_registers(connection_manager_.get(), data_info.register_address,
				num_registers, rp_registers.get());
			if (rc != num_registers)
			{
#ifdef _DEBUG
				std::cerr << "Read input registers " << std::hex << std::showbase << data_info.register_address << " failed." << std::endl;
#endif // DEBUG			
			}
			AssignRegisterValue(rd, rp_registers);
			rd->result = 0;
			break;
		default:
			throw std::invalid_argument("DriverWorker::ReadData() -> Unsupported data zone.");
		}
		return rd;
	}

	int DriverWorker::GetNumberOfRegister(DataType datatype)
	{
		switch (datatype)
		{
		case DataType::DB:
			return 2;
		case DataType::DF:
			return 2;
		case DataType::DUB:
			return 2;
		default:
			return 1;
		}
	}

	void DriverWorker::AssignRegisterValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint16_t> registers)
	{
		switch (data_info->data_type)
		{
		case DataType::DB:
		case DataType::DUB:
			data_info->int_value = registers.get()[0] + (registers.get()[1] << 16);
			break;
		case DataType::DF:
			switch (data_info->float_decode)
			{
			case FloatDecode::ABCD:
				data_info->float_value = modbus_get_float_abcd(registers.get());
				break;
			case FloatDecode::DCBA:
				data_info->float_value = modbus_get_float_dcba(registers.get());
				break;
			case FloatDecode::BADC:
				data_info->float_value = modbus_get_float_badc(registers.get());
				break;
			case FloatDecode::CDAB:
				data_info->float_value = modbus_get_float_cdab(registers.get());
				break;
			default:
				throw std::invalid_argument("Unsupported float decode.");
			}
			break;
		case DataType::WB:
		case DataType::WUB:
			data_info->int_value = registers.get()[0];
			break;
		default:
			throw std::invalid_argument("Unsupported data type.");
		}
	}

	void DriverWorker::AssignBitValue(std::shared_ptr<DataInfo> data_info, std::shared_ptr<uint8_t> bits)
	{
		data_info->int_value = bits.get()[0];
	}

}