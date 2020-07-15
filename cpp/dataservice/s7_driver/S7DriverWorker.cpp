#include "pch.h"
#include <iostream>
#include "S7DriverWorker.h"


int goiot::S7DriverWorker::OpenConnection()
{
	connection_manager_.reset(new TS7Client);
	std::string address = "192.168.0.13";
	int rack = 0;
	int slot = 0;
	int res = connection_manager_->ConnectTo(address.c_str(), rack, slot);
	if (res != 0)
	{
		std::cerr << "S7 Connection failed: " << CliErrorText(res) << std::endl;
	}
	else
	{
		std::cout << "S7 Connected to: " << address << " Rack=" << rack << ", Slot=" << slot << std::endl;
	}
	return res;
}

void goiot::S7DriverWorker::CloseConnection()
{
	connection_manager_.reset();
	connected_ = false;
}