#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/chrono.hpp>

class Barrier
{
public:
	Barrier(size_t number);
    
	void wait();
	bool timedWait(boost::chrono::microseconds timeout);
    
private:
	bool step();

	boost::interprocess::interprocess_condition condition;
	boost::interprocess::interprocess_mutex     mutex;
	size_t number;
	size_t counter;
};