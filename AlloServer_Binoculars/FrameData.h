//#ifndef FRAMEDATA_H
//#define FRAMEDATA_H
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

struct FrameData
{
    FrameData()
    :  shutdownServer(false)//, showBuffer1(false), showBuffer2(false)
    {}

    //unsigned char pixels[1024*640*3];
    unsigned char pixels[1024*576*4];
	//unsigned char pixels[1024*1024*3];
    //unsigned char pixels[2048*2048*3];
    //unsigned char pixels1[1];
    
    boost::interprocess::interprocess_mutex mutex;
//    boost::interprocess::interprocess_semaphore semMutex;
    bool shutdownServer;
    //sem_t *semaphore;
  //bool showBuffer1, showBuffer2;
};
FrameData *sharedData = NULL;

//#endif
