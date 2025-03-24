#include "pch.h"
#include <iostream>
#include <cassert>
#include "FpDriver.h"

namespace goiot 
{
    RESULT_DSAPI FpDriver::InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info)
    {
        return 0;
    }

    RESULT_DSAPI FpDriver::UnitDriver()
    {
        return 0;
    }

    RESULT_DSAPI FpDriver::AsyncWrite(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id)
    {
        throw std::runtime_error("Not implemented.");
    }

    RESULT_DSAPI FpDriver::AsyncRead(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id, std::function<void(std::vector<DataInfo>&&, uint64_t)> read_callback)
    {
        throw std::runtime_error("Not implemented.");
    }
}
