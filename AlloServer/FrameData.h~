//#ifndef FRAMEDATA_H
//#define FRAMEDATA_H
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

struct FrameData
{
    FrameData()
    :  shutdownServer(false)//, showBuffer1(false), showBuffer2(false)
    {}
//    unsigned char buffer1[1280*720*3];
//    unsigned char buffer2[1280*720*3];
    unsigned char pixels[1280*720*3];
    //unsigned char pixels1[1];
    
    boost::interprocess::interprocess_mutex mutex;
//    boost::interprocess::interprocess_semaphore semMutex;
    bool shutdownServer;
    //sem_t *semaphore;
  //bool showBuffer1, showBuffer2;
};
FrameData *sharedData = NULL;

//#endif
