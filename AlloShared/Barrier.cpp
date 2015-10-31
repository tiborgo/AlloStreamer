#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "Barrier.hpp"

Barrier::Barrier(size_t number)
    :
	number(number),
    counter(number)
{
}

void Barrier::wait()
{
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
	if (!step())
	{
		condition.wait(lock);
	}
}

bool Barrier::timedWait(boost::chrono::microseconds timeout)
{
	if (!mutex.timed_lock(boost::get_system_time() + boost::posix_time::microseconds(timeout.count())))
	{
		return false;
	}
	else
	{
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex,
			                                                                           boost::interprocess::accept_ownership);

		if (counter == 1)
		{
			counter = number;
			condition.notify_all();
			return true;
		}
		else
		{
			counter--;
			if (!condition.timed_wait(lock,
				                      boost::get_system_time() + boost::posix_time::microseconds(timeout.count())))
			{
				// No other thread reached barrier while we were waiting -> restore barrier and exit
				counter++;
				return false;
			}
			else
			{
				return true;
			}
		}
	}
}

void Barrier::reset()
{
	// Hacky solution indeed
	// That's not possible unfortunately since the mutex is locked and abandoned
	//mutex.~interprocess_mutex();
	void* mutexAddr = &mutex;
	void* conditionAddr = &condition;
	memset(mutexAddr, 0, sizeof(boost::interprocess::interprocess_mutex));
	memset(conditionAddr, 0, sizeof(boost::interprocess::interprocess_condition));
	new (mutexAddr)     boost::interprocess::interprocess_mutex;
	new (conditionAddr)boost::interprocess::interprocess_condition;
}

bool Barrier::step()
{
	counter--;
	if (counter == 0)
	{
		condition.notify_all();
		counter = number;
		return true;
	}
	else
	{
		return false;
	}
}