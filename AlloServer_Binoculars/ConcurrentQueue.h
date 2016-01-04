#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <stdio.h>
#include <queue>

template<typename Data>
class ConcurrentQueue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
    FILE* queueFile;
public:
  ConcurrentQueue()
  {
	  queueFile = fopen(ROOT_DIR "/Logs/queue.log", "w");
  }
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        //fprintf(queueFile, "1Pushing, queue size: %i\n", the_queue.size());
        //fflush(queueFile);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }
    
    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }
    
    bool tryPop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }
    
    void waitAndPop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }
      
        popped_value=the_queue.front();
        //fprintf(queueFile, "2Popping, queue size: %i\n", the_queue.size());
        //fflush(queueFile);
        the_queue.pop();
    }
    int get_size()
    {
        return the_queue.size();
    }
    
};
