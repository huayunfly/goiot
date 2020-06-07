#include "pch.h"
#include <mutex>
#include "DriverWorker.h"

namespace goiot
{
	void DriverWorker::OpenConnection()
	{
		connection_manager_.reset(modbus_new_rtu(connection_details_.port.c_str(),
			connection_details_.baud, connection_details_.parity, connection_details_.data_bit, connection_details_.stop_bit));

	}

	void DriverWorker::CloseConnection()
	{

	}

	void DriverWorker::DoWork()
	{
		std::call_once(connection_init_flag_, &DriverWorker::OpenConnection, this);

	}
}