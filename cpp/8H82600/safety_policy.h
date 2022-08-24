
#ifndef SAFETYPOLICY_H
#define SAFETYPOLICY_H

#include <vector>
#include <deque>
#include <mutex>
#include <future>
#include <thread>


class SafetyPolicy
{
public:
    SafetyPolicy();
    ~SafetyPolicy();

    SafetyPolicy(const SafetyPolicy&) = delete;
    SafetyPolicy& operator=(const SafetyPolicy&) = delete;

    template<typename Func>
    std::future<void> PostTask(Func f)
    {
        std::packaged_task<void()> task(f);
        std::future<void> res = task.get_future();
        std::lock_guard<std::mutex> lk(m_);
        tasks_.push_back(task);
        return res;
    }

private:
    void RunTask();

private:
    std::vector<std::thread> threads_;
    std::mutex m_;
    bool run_;
    std::deque<std::packaged_task<void()> > tasks_;

};

#endif // SAFETYPOLICY_H
