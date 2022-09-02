#include "safety_policy.h"
#include <chrono>
#include <qlogging.h>


SafetyPolicy::SafetyPolicy(goiot::DataManager& data_manager) :
    run_(true), data_manager_(data_manager)
{

}

SafetyPolicy::~SafetyPolicy()
{
    Stop();
}

void SafetyPolicy::Start()
{
    threads_.emplace_back(std::thread(&SafetyPolicy::RunTask, this));
}

void SafetyPolicy::Stop()
{
    run_ = false;
    for (auto& thread: threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void SafetyPolicy::RunTask()
{
    while(run_)
    {
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m_);
            if (tasks_.empty())
            {
                continue;
            }
            task = std::move(tasks_.front());
            tasks_.pop_front();
        }
        try
        {
            task();
        }
        catch (...)
        {
            qCritical("SafetyPolicy::RunTask() exception.");
        }
    }
}

void SafetyPolicy::TaskCheckExperimentState()
{
    const double PFC_EXPERIMENT_RUN_LIMIT = 4.0; // barA
    const double PFC_HIGH_LIMIT = 51.0; // barA
    const double PFC_HHIGH_LIMIT = 54.0; // barA
    std::vector<std::string> pfc_ids = {"mfcpfc.11.pv", "mfcpfc.12.pv",
                                       "mfcpfc.13.pv", "mfcpfc.14.pv",
                                       "mfcpfc.15.pv", "mfcpfc.16.pv",
                                       "mfcpfc.17.pv", "mfcpfc.18.pv",
                                       "mfcpfc.19.pv", "mfcpfc.20.pv",
                                       "mfcpfc.21.pv", "mfcpfc.22.pv",
                                       "mfcpfc.23.pv", "mfcpfc.24.pv",
                                       "mfcpfc.25.pv", "mfcpfc.26.pv"};
    std::vector<goiot::DataInfo> pfc_list =
            data_manager_.ReadDataCache(pfc_ids);
    std::vector<std::string> proximity_sensor_ids = {"plc.1.di1_17", "plc.1.di1_18",
                                                    "plc.1.di1_19", "plc.1.di1_20",
                                                    "plc.1.di1_21", "plc.1.di1_22",
                                                    "plc.1.di1_23", "plc.1.di1_24",
                                                    "plc.1.di1_25", "plc.1.di1_26",
                                                    "plc.1.di1_27", "plc.1.di1_28",
                                                    "plc.1.di1_29", "plc.1.di1_30",
                                                    "plc.1.di1_31", "plc.1.di1_32"};
    std::vector<goiot::DataInfo> proximity_sensor_list =
            data_manager_.ReadDataCache(proximity_sensor_ids);
    std::vector<std::string> experiment_run_ids = {"plc.1.reactor_1_run", "plc.1.reactor_2_run",
                                                   "plc.1.reactor_3_run", "plc.1.reactor_4_run",
                                                   "plc.1.reactor_5_run", "plc.1.reactor_6_run",
                                                   "plc.1.reactor_7_run", "plc.1.reactor_8_run",
                                                   "plc.1.reactor_9_run", "plc.1.reactor_10_run",
                                                   "plc.1.reactor_11_run", "plc.1.reactor_12_run",
                                                   "plc.1.reactor_13_run", "plc.1.reactor_14_run",
                                                   "plc.1.reactor_15_run", "plc.1.reactor_16_run"
                                                  };
    std::vector<std::string> alarm_pfc_high_ids = {"plc.1.alm_pfc_1_high", "plc.1.alm_pfc_2_high",
                                                "plc.1.alm_pfc_3_high", "plc.1.alm_pfc_4_high",
                                                "plc.1.alm_pfc_5_high", "plc.1.alm_pfc_6_high",
                                                "plc.1.alm_pfc_7_high", "plc.1.alm_pfc_8_high",
                                                "plc.1.alm_pfc_9_high", "plc.1.alm_pfc_10_high",
                                                "plc.1.alm_pfc_11_high", "plc.1.alm_pfc_12_high",
                                                "plc.1.alm_pfc_13_high", "plc.1.alm_pfc_14_high",
                                                "plc.1.alm_pfc_15_high", "plc.1.alm_pfc_16_high"
                                               };
    std::vector<std::string> alarm_pfc_hhigh_ids = {"plc.1.alm_pfc_1_hhigh", "plc.1.alm_pfc_2_hhigh",
                                                "plc.1.alm_pfc_3_hhigh", "plc.1.alm_pfc_4_hhigh",
                                                "plc.1.alm_pfc_5_hhigh", "plc.1.alm_pfc_6_hhigh",
                                                "plc.1.alm_pfc_7_hhigh", "plc.1.alm_pfc_8_hhigh",
                                                "plc.1.alm_pfc_9_hhigh", "plc.1.alm_pfc_10_hhigh",
                                                "plc.1.alm_pfc_11_hhigh", "plc.1.alm_pfc_12_hhigh",
                                                "plc.1.alm_pfc_13_hhigh", "plc.1.alm_pfc_14_hhigh",
                                                "plc.1.alm_pfc_15_hhigh", "plc.1.alm_pfc_16_hhigh"
                                               };
    std::vector<goiot::DataInfo> write_data_list;
    for (std::size_t i = 0; i < pfc_list.size(); i++)
    {
        auto& pfc = pfc_list.at(i);
        auto& sensor = proximity_sensor_list.at(i);
        if (pfc.id.empty() || sensor.id.empty())
        {
            continue;
        }
        double fvalue = pfc.int_value * pfc.ratio;
        uint8_t state = sensor.byte_value;
        if (fvalue > PFC_HHIGH_LIMIT)
        {
            std::vector<goiot::DataInfo> pfc_alm_hhigh_list = data_manager_.ReadDataCache(
                        std::vector<std::string> {alarm_pfc_hhigh_ids.at(i)});
            assert(!pfc_alm_hhigh_list.at(0).id.empty());
            pfc_alm_hhigh_list.at(0).byte_value = 1;
            write_data_list.push_back(pfc_alm_hhigh_list.at(0));
        }
        else if (fvalue > PFC_HIGH_LIMIT)
        {
            std::vector<goiot::DataInfo> pfc_alm_high_list = data_manager_.ReadDataCache(
                        std::vector<std::string> {alarm_pfc_high_ids.at(i)});
            assert(!pfc_alm_high_list.at(0).id.empty());
            pfc_alm_high_list.at(0).byte_value = 1;
            write_data_list.push_back(pfc_alm_high_list.at(0));
        }
        else if (fvalue > PFC_EXPERIMENT_RUN_LIMIT && state == 1)
        {
            std::vector<goiot::DataInfo> run_list = data_manager_.ReadDataCache(
                        std::vector<std::string> {experiment_run_ids.at(i)});
            assert(!run_list.at(0).id.empty());
            if (run_list.at(0).byte_value == 0)
            {
                run_list.at(0).byte_value = 1;
                write_data_list.push_back(run_list.at(0)); // ready to write
            }
        }
    }
    if (write_data_list.size() > 0)
    {
        for (auto& data_info : write_data_list)
        {
            data_info.data_flow_type = goiot::DataFlowType::ASYNC_WRITE;
            data_info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock().now().time_since_epoch()).count() / 1000.0;
            data_info.result = 0;
        }
        data_manager_.WriteDataAsync(write_data_list);
    }
}
