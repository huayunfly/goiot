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
#include <stdexcept>

namespace goiot
{
	class Full : std::runtime_error
	{
	public:
		virtual const char* what() const noexcept
		{
			return "Queue full.";
		}
	};

	class Empty : std::runtime_error
	{
	public:
		virtual const char* what() const noexcept
		{
			return "Queue empty.";
		}
	};

	template<typename T>
	class ThreadSafeQueue
	{
	public:
		static const long long MAX_MILLSECONDES = 10000000;
		// Create a queue object with a given maximum size.
		// If maxsize is <= 0, the queue size is infinite.
		ThreadSafeQueue(std::size_t maxsize = 0) : maxsize_(maxsize), unfinished_tasks_(0)
		{

		}

		ThreadSafeQueue(const ThreadSafeQueue& other)
		{
			std::lock_guard<std::mutex> lk(other.mut_);		
			data_queue_ = other.data_queue_;
			maxsize_ = other.maxsize_;
		}

 		ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

		// Put an item into the queue.
		void Put(T new_value, bool block = true, std::chrono::milliseconds timeout = std::chrono::milliseconds(MAX_MILLSECONDES))
		{
			std::unique_lock<std::mutex> lk(mut_);
			if (maxsize_ > 0)
			{
				if (!block) // not block
				{
					if (QSize_() >= maxsize_)
					{
						throw Full();
					}
				}
				else if (timeout >= std::chrono::milliseconds(MAX_MILLSECONDES)) // block with infinite timeout
				{
					not_full_.wait(lk, [this] { return QSize_() < maxsize_; });
				}
				else if (timeout < std::chrono::milliseconds(0))
				{
					throw std::invalid_argument("'timeout' must be a non-negative number");
				}
				else
				{
					bool not_full = not_full_.wait_for(lk, timeout, [this] { return QSize_() < maxsize_; }); // the return value of the predict when woken
					if (!not_full)
					{
						throw Full();
					}
				}
			}
			data_queue_.push(new_value);
			unfinished_tasks_ += 1;
			not_empty_.notify_one();
		}

		void Close()
		{
			T value(nullptr);
			Put(value);
		}

		// Put an item into the queue without blocking.
		void PutNoWait(T new_value)
		{
			Put(new_value, false);
		}

		// Get an item into the queue without blocking.
		bool GetNoWait(T& value)
		{
			return Get(value, false);
		}

		// Get an item into the queue without blocking.
		std::shared_ptr<T> GetNoWait()
		{
			return Get(false);
		}

		bool TryGet(T& value)
		{
			std::lock_guard<std::mutex> lk(mut_);
			if (data_queue_.empty())
			{
				return false;
			}
			value = data_queue_.front();
			data_queue_.pop();
			return true;
		}

		std::shared_ptr<T> TryGet()
		{
			std::lock_guard<std::mutex> lk(mut_);
			if (data_queue_.empty())
			{
				return std::shared_ptr<T>();
			}
			std::shared_ptr<T> res(std::make_shared<T>(data_queue_.front()));
			data_queue_.pop();
			return res;
		}

		// Remove and return an item from the queue.
		void Get(T& value, bool block = true, std::chrono::milliseconds timeout = std::chrono::milliseconds(MAX_MILLSECONDES))
		{
			std::unique_lock<std::mutex> lk(mut_);
			if (!block) // no block
			{
				if (QSize_() == 0)
				{
					throw Empty();
				}
			}
			else if (timeout >= std::chrono::milliseconds(MAX_MILLSECONDES)) // block but with infinite timeout
			{
				not_empty_.wait(lk, [this] { return !data_queue_.empty(); });
			}
			else if (timeout < std::chrono::milliseconds(0))
			{
				throw std::invalid_argument("'timeout' must be a non-negative number");
			}
			else
			{
				bool not_empty = not_empty_.wait_for(lk, timeout, [this] { return !data_queue_.empty(); });
				if (!not_empty)
				{
					throw Empty();
				}
			}
			value = data_queue_.front();
			data_queue_.pop();
			not_full_.notify_one();
		}

		// Remove and return an item from the queue.
		std::shared_ptr<T> Get(bool block = true, std::chrono::milliseconds timeout = MAX_MILLSECONDES)
		{
			std::unique_lock<std::mutex> lk(mut_);
			if (!block) // no block
			{
				if (QSize_() == 0)
				{
					throw Empty();
				}
			}
			else if (timeout >= MAX_MILLSECONDES) // block but with infinite timeout
			{
				not_empty_.wait(lk, [this] { return !data_queue_.empty(); });
			}
			else if (timeout < std::chrono::milliseconds(0))
			{
				throw std::invalid_argument("'timeout' must be a non-negative number");
			}
			else
			{
				bool not_empty = not_empty_.wait_for(lk, timeout, [this] { return !data_queue_.empty(); });
				if (!not_empty)
				{
					throw Empty();
				}
			}
			std::shared_ptr<T> res(std::make_shared<T>(data_queue_.front()));
			return res;
		}

		//  Return True if the queue is empty, False otherwise (not reliable!).
		//  Be aware that either approach risks a race condition where a queue 
		//  can grow before the result of Empty() or qsize() can be used.
		bool Empty() const
		{
			std::lock_guard<std::mutex> lk(mut_);
			return data_queue_.empty();
		}

		//  Return True if the queue is full, False otherwise (not reliable!).
		//  Be aware that either approach risks a race condition where a queue 
		//  can grow before the result of Full() or qsize() can be used.
		bool Full() const
		{
			std::lock_guard<std::mutex> lk(mut_);
			return maxsize_ > 0 && data_queue_.size() >= maxsize_;
		}

		std::size_t QSize() const
		{
			std::lock_guard<std::mutex> lk(mut_);
			return QSize_();
		}

		// member typedefs provided through inheriting from std::iterator
		class iterator : public std::iterator<
			std::input_iterator_tag,   // iterator_category
			long,                      // value_type
			long,                      // difference_type
			const long*,               // pointer
			long                       // reference
		> {
			long num = 4;
		public:
			explicit iterator(long _num = 0) : num(_num) {}
			iterator& operator++() { num = 7 >= 4 ? num + 1 : num - 1; return *this; }
			iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
			bool operator==(iterator other) const { return num == other.num; }
			bool operator!=(iterator other) const { return !(*this == other); }
			reference operator*() const { return num; }
		};

		iterator begin() { return iterator(4); }
		iterator end() { return iterator(7 >= 4 ? 7 + 1 : 7 - 1); }

	private:
		std::size_t QSize_() const
		{
			return data_queue_.size();
		}

	private:
		mutable std::mutex mut_; // The mutex must be mutable.
		std::size_t maxsize_;
		int unfinished_tasks_;
		std::queue<T> data_queue_;
		std::condition_variable not_empty_;
		std::condition_variable not_full_;
	};
}


