#pragma once

#include <boost/cstdint.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <libavutil/pixfmt.h>
#include <boost/chrono/system_clocks.hpp>

#include "Allocator.h"

class Frame
{

public:
	typedef boost::interprocess::offset_ptr<Frame> Ptr;
    
    boost::uint32_t                              getWidth();
    boost::uint32_t                              getHeight();
    AVPixelFormat                                getFormat();
    boost::chrono::system_clock::time_point      getPresentationTime();
    void*                                        getPixels();
    boost::interprocess::interprocess_mutex&     getMutex();
    boost::interprocess::interprocess_condition& getNewPixelsCondition();
    void*                                        getUserData();
    
    void setPresentationTime(boost::chrono::system_clock::time_point presentationTime);
    
    static Frame* create(boost::uint32_t                         width,
                         boost::uint32_t                         height,
                         AVPixelFormat                           format,
                         boost::chrono::system_clock::time_point presentationTime,
                         void*                                   userData,
                         Allocator&                              allocator);
    static void   destroy(Frame* Frame);
    
protected:
    Frame(boost::uint32_t                         width,
          boost::uint32_t                         height,
          AVPixelFormat                           format,
          boost::chrono::system_clock::time_point presentationTime,
          void*                                   pixels,
          void*                                   userData,
          Allocator&                              allocator);
    ~Frame();
    
    Allocator&                                  allocator;
    boost::uint32_t                             width;
    boost::uint32_t                             height;
    AVPixelFormat                               format;
    boost::chrono::system_clock::time_point     presentationTime;
    boost::interprocess::offset_ptr<void>       pixels;
    boost::interprocess::interprocess_mutex     mutex;
    boost::interprocess::interprocess_condition newPixelsCondition;
    boost::interprocess::offset_ptr<void>       userData;
};