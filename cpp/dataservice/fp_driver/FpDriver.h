// @author: Yun Hua (huayunfly at 126.com)
// @version: 0.1 2024.10.11
#pragma once

#include <json/json.h>
#include "../dataservice/driver_base.h"
#include "./FpDriverWorker.h"

namespace goiot
{
    class FpDriver : public DriverBase
    {
    public:
        FpDriver() : _driver_worker(), _id(), _worker_ready(false)
        {

        }

        FpDriver(const FpDriver&) = delete;
        FpDriver& operator=(const FpDriver&) = delete;

        // DriverBase interface
        virtual RESULT_DSAPI GetDescription(std::string& description) override
        {
            description = "fpplc";
            return 0;
        }

        virtual RESULT_DSAPI GetID(std::string& id) override
        {
            id = _id;
            return 0;
        }

        virtual RESULT_DSAPI InitDriver(const std::string& config, std::shared_ptr<ThreadSafeQueue<std::shared_ptr<std::vector<DataInfo>>>> response_queue, std::function<void(const DataInfo&)> set_data_info) override;
        virtual RESULT_DSAPI UnitDriver() override;
        virtual RESULT_DSAPI AsyncWrite(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id) override;
        virtual RESULT_DSAPI AsyncRead(const std::vector<DataInfo>& data_info_vec, uint64_t trans_id, std::function<void(std::vector<DataInfo>&&, uint64_t)> read_callback) override;

    private:
        /// <summary>
        /// Parse driver ID, device data_info, connection string.
        /// </summary>
        /// <param name="config">Configuration json:
        ///     {
        //        "id": "sqlite",
        //        "name" : "sqlite3",
        //        "type" : "sqlite",
        //        "nodes" : [
        //         {
        //              "address": "expinfo",
        //              "data" : [
        //              {
        //                   "id": "recordpath",
        //                   "name" : "recordpath",
        //                   "register" : "RWSTR"
        //              }
        //            ]
        //          }
        //        ]
        //      }
        /// </param>
        /// <param name="connection_info">ConnectionInfo</param>
        /// <param name="data_map">A data map reference.</param>
        /// <returns>0: succeeded, otherwise failed.</returns>
        int ParseConfig(const std::string& config,
            ConnectionInfo& connection_info, std::map<std::string, DataInfo>& data_map);

    private:
        std::unique_ptr<FpDriverWorker> _driver_worker;
        std::string _id;
        bool _worker_ready;
    };
}



