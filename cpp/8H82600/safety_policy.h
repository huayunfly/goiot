
#ifndef SAFETYPOLICY_H
#define SAFETYPOLICY_H

#include <vector>
#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <memory>
#include "data_manager.h"

class SafetyPolicy
{
public:
    SafetyPolicy(goiot::DataManager& data_manager);
    ~SafetyPolicy();

    SafetyPolicy(const SafetyPolicy&) = delete;
    SafetyPolicy& operator=(const SafetyPolicy&) = delete;

    template<typename Func>
    std::future<void> PostTask(Func f)
    {
        std::packaged_task<void()> task(f);
        std::future<void> res = task.get_future();
        std::lock_guard<std::mutex> lk(m_);
        tasks_.push_back(std::move(task));
        return res;
    }

    // Start executing task thread.
    void Start();

    // Stop executing task thread.
    void Stop();

    // Check PFC pressure high limit and notify PLC.
    // Check PFC pressure hhigh limit and notify PLC.
    // Check PFC pressure, reactor cylinder state and set experiment start sign.
    void TaskCheckExperimentState();

private:
    // Run task thread routine.
    void RunTask();

private:
    std::vector<std::thread> threads_;
    std::mutex m_;
    bool run_;
    std::deque<std::packaged_task<void()> > tasks_;
    goiot::DataManager& data_manager_;
};

#endif // SAFETYPOLICY_H
