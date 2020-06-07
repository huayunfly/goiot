// @purpose: A thread safe queue interface, from C++ concurrency
// @author: huayunfly@126.com
// @date: 2020.06.07
// @copyright: GNU
// @version: 0.1

#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace goiot
{
	template<typename T>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue()
		{

		}

		ThreadSafeQueue(const ThreadSafeQueue& other)
		{
			std::lock_guard<std::mutex> lk(other.mut);
			data_queue = other.data_queue;
		}

 		ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

		void Push(T new_value)
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(new_value);
			data_cond.notify_one();
		}
		bool TryPop(T& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
			{
				return false;
			}
			value = data_queue.front();
			data_queue.pop();
			return true;
		}

		std::shared_ptr<T> TryPop()
		{
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty())
			{
				return false;
			}
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}

		void WaitAndPop(T& value)
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] { return !data_queue.empty(); });
			value = data_queue.front();
			data_queue.pop();
		}

		std::shared_ptr<T> WaitAndPop()
		{
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] { return !data_queue.empty(); });
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			return res;
		}

		bool Empty() const
		{
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}

	private:
		mutable std::mutex mut; // The mutex must be mutable.
		std::queue<T> data_queue;
		std::condition_variable data_cond;
	};
}


