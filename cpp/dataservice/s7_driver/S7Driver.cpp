#include "pch.h"
#include <iostream>
#include "S7Driver.h"

RESULT_DSAPI goiot::S7Driver::InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info)
{
    if (response_queue == nullptr)
    {
        throw std::invalid_argument("Parameter response_queue is null.");
    }
    ConnectionInfo connection_details;
    std::map<std::string, DataInfo> data_map;

    driver_worker_.reset(new S7DriverWorker(connection_details, std::move(data_map), response_queue));
    int return_code = driver_worker_->OpenConnection();
    std::cout << "S7Driver::InitDriver() returns " << return_code << std::endl;
    return 0;
}

RESULT_DSAPI goiot::S7Driver::UnitDriver()
{
	return RESULT_DSAPI();
}

RESULT_DSAPI goiot::S7Driver::AsyncWrite(const std::vector<DataInfo>& data_info_vec)
{
	return RESULT_DSAPI();
}

RESULT_DSAPI goiot::S7Driver::AsyncRead(const std::vector<std::string> id_vec, std::function<void(std::vector<DataInfo>&&)> read_callback)
{
	return RESULT_DSAPI();
}
