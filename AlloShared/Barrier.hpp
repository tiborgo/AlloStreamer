#pragma once

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

class Barrier
{
public:
	Barrier(size_t number);
    
	void wait();
    
private:
	boost::interprocess::interprocess_condition condition;
	boost::interprocess::interprocess_mutex     mutex;
	size_t number;
	size_t counter;
};