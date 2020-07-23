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
	    int read_db_ret = connection_manager_->DBRead(1, 0, 4, &data_vec.at(0));
		//int size = 16;
	    //read_db_ret = connection_manager_->DBGet(1, &data_vec.at(0), &size); // errCliFunNotAvailable
		data_vec.at(0) = 0xfe;
		int write_db_ret = connection_manager_->DBWrite(1, 12, 1, &data_vec.at(0));
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
		refresh_ = true;
		threads_.emplace_back(&S7DriverWorker::Refresh, this);
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
						data_info.second.data_zone, data_info.second.float_decode, data_info.second.int_value, data_info.second.float_value,
						data_info.second.char_value, std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0);
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
			catch (const std::exception & e) {
				std::cerr << "EXCEPTION: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "EXCEPTION (unknown)" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	// Dispatch worker deals with the in_queue request, which may read/write message to hardware.
	// The return data are put into the out_queue.
	void Request_Dispatch()
	{

	}

	// Dispatch worker deals with the out_queue request, which may trasnfer data to the DataService.
	void Response_Dispatch()
	{

	}


	// Puts asynchronous read request to the in_queue.
	void AsyncRead(const std::vector<std::string> var_names,
		const std::vector<std::string> var_ids, int trans_id)
	{

	}

	// Puts asynchronous write request to the out_queue.
	void AsyncWrite(const std::vector<DataInfo>& data_info_vec, int trans_id)
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