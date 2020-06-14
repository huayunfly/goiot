// DriverWorker implementation.
// Modbus is working on master to multiple slaves mode.
// Sending command (master -> slave)
// 1byte slave address, 1 byte function code, 0~252 byte data, 2 byte CRC
// 00h: the broadcast address
//
// Write coil ON/OFF, ON: 1, OFF: 0
//
// MODBUS RTU explanation: https://blog.csdn.net/hejinjing_tom_com/article/details/49738209

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
		modbus_set_debug(connection_manager_.get(), TRUE);

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
			std::clog << "Connection failed: " << modbus_strerror(errno) << std::endl;
			return ENOTCONN;
		}
		uint32_t new_response_to_sec;
		uint32_t new_response_to_usec;
		modbus_get_response_timeout(connection_manager_.get(), &new_response_to_sec, &new_response_to_usec);
		
		const int SERVER_ID = 1;
		modbus_set_slave(connection_manager_.get(), SERVER_ID);
	}

	void DriverWorker::CloseConnection()
	{
		connection_manager_ = nullptr;
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
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	}

	void DriverWorker::Request_Dispatch()
	{

	}

	void DriverWorker::Response_Dispatch()
	{

	}
}