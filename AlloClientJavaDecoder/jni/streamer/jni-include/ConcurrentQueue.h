#include <stdio.h>
#include <queue>
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "ConcurrentQueue", __VA_ARGS__);

template<typename Data>
class ConcurrentQueue
{
private:
    std::queue<Data> the_queue;
    //mutable boost::mutex the_mutex;
    //boost::condition_variable the_condition_variable;
    sem_t sem;
    sem_t mutex;
public:
    ConcurrentQueue()
    {
    	sem_init(&sem, 0, 0);
    	sem_init(&mutex, 0, 1);
    }
    void push(Data const& data)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
    	//sem_wait(&mutex);
    	//printf("1pushing...");
        the_queue.push(data);
        //printf("1pushed.");
        //lock.unlock();
        //the_condition_variable.notify_one();

        //Add one to semaphore
        sem_post(&sem);
        //sem_post(&mutex);
    }

    bool empty() const
    {
        //boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }

        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
    	//sem_wait(&mutex);
        while(the_queue.empty())
        {
            //the_condition_variable.wait(lock);
        	//wait on semaphore
        	sem_wait(&sem);
        }

        popped_value=the_queue.front();
        //printf("popping...");
        the_queue.pop();
        //printf("popped! size is: %i", the_queue.size());
        //sem_post(&mutex);
    }
    int get_size()
    {
        return the_queue.size();
    }

};
