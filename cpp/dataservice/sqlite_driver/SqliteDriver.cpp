#include "pch.h"
#include "SqliteDriver.h"

namespace goiot
{
	RESULT_DSAPI SqliteDriver::InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info)
	{
		return ENOTCONN;
	}

	RESULT_DSAPI SqliteDriver::UnitDriver()
	{
		return ENOTCONN;
	}

	RESULT_DSAPI SqliteDriver::AsyncWrite(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id)
	{
		return ENOTCONN;
	}

	RESULT_DSAPI SqliteDriver::AsyncRead(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id, std::function<void(std::vector<DataInfo>&&, uint64_t)> read_callback)
	{
		return ENOTCONN;
	}

	int SqliteDriver::ParseConfig(const std::string& config,
		ConnectionInfo& connection_info, std::map<std::string, DataInfo>& data_map)
	{
		return 0;
	}
}
