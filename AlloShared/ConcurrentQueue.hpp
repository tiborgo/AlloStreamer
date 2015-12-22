#pragma once

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

template<typename Data>
class ConcurrentQueue
{
private:
	std::queue<Data> queue;
	mutable boost::mutex mutex;
	boost::condition_variable conditionVariable;
	bool isClosed_;

public:
	ConcurrentQueue() : isClosed_(false)
	{

	}

	void push(Data const& data)
	{
		boost::mutex::scoped_lock lock(mutex);
		queue.push(data);
		lock.unlock();
		conditionVariable.notify_one();
	}

	bool empty() const
	{
		boost::mutex::scoped_lock lock(mutex);
		return queue.empty();
	}

	bool tryPop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(mutex);
		if (isClosed_ || queue.empty())
		{
			return false;
		}

		popped_value = queue.front();
		queue.pop();
		return true;
	}

	bool waitAndPop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(mutex);
		while (!isClosed_ && queue.empty())
		{
			conditionVariable.wait(lock);
		}

		if (isClosed_)
		{
			return false;
		}

		popped_value = queue.front();
		queue.pop();
		return true;
	}

	void close()
	{
		boost::mutex::scoped_lock lock(mutex);
        std::queue<Data> emptyQueue;
        queue.swap(emptyQueue); // clears queue
		isClosed_ = true;
		conditionVariable.notify_all();
	}

	size_t size()
	{
		boost::mutex::scoped_lock lock(mutex);
		return queue.size();
	}

	bool isClosed()
	{
		return isClosed_;
	}
};
