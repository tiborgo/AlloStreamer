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
	counter--;
	if (counter == 0)
	{
		condition.notify_all();
		counter = number;
	}
	else
	{
		condition.wait(lock);
	}
}