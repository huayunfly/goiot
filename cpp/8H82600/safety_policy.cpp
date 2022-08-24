#include "safety_policy.h"
#include <qlogging.h>

SafetyPolicy::SafetyPolicy() : run_(true)
{
    threads_.emplace_back(std::thread(&SafetyPolicy::RunTask, this));
}

SafetyPolicy::~SafetyPolicy()
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
