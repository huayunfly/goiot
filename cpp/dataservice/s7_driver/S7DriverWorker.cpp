#include "pch.h"
#include <iostream>
#include <vector>
#include "S7DriverWorker.h"

namespace goiot
{
	int S7DriverWorker::OpenConnection()
	{
		connection_manager_.reset(new TS7Client,
			[](TS7Client* p) { p->Disconnect(); std::cout << "free S7 ptr."; });
		int res = connection_manager_->ConnectTo(connection_details_.port.c_str(),
			connection_details_.rack, connection_details_.slot);
		if (res != 0)
		{
			std::cerr << "S7 Connection failed: " << CliErrorText(res) << std::endl;
			return ENOTCONN;
		}
		else
		{
			std::cout << "S7 Connected to: " << connection_details_.port << " Rack=" <<
				connection_details_.rack << ", Slot=" << connection_details_.slot << std::endl;
		}
		S7CPUStatus status = PLCStatus();  // Todo: CPU run => connected_ ?
		ListBlocks();
		connected_ = true;

		std::vector<byte> data_vec(1000);
	    int read_db_ret = connection_manager_->DBRead(1, 0, 10, &data_vec.at(0));
		int size = 100;
	    read_db_ret = connection_manager_->DBGet(1, &data_vec.at(0), &size); // error
		return res;
	}

	void S7DriverWorker::CloseConnection()
	{
		connection_manager_.reset();
		connected_ = false;
#ifdef _DEBUG
		std::cout << "S7 driver worker closed connection.";
#endif // _DEBUG
	}

	// Spawns working threads.
	void S7DriverWorker::Start()
	{

	}

	// Stops and waits the working threads completed.
	void S7DriverWorker::Stop()
	{

	}

	// Refreshs the data reading or writing requests into in_queue.
	void S7DriverWorker::Refresh()
	{

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
}