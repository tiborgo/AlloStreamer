#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>
#include <map>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"
#include "RTSPCubemapSource.hpp"

class ALLORECEIVER_API H264CubemapSource : public RTSPCubemapSource
{
public:
	virtual void setOnNextCubemap(std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)>& callback);
    virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int, u_int8_t, size_t)>&    callback);
    virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int, u_int8_t, size_t)>&    callback);
    
    H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks,
                      AVPixelFormat format);

protected:
	std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)> onNextCubemap;
    std::function<void (CubemapSource*, int, u_int8_t, size_t)>    onDroppedNALU;
    std::function<void (CubemapSource*, int, u_int8_t, size_t)>    onAddedNALU;
    
private:
    void getNextFramesLoop();
    void getNextCubemapLoop();
    void sinkOnDroppedNALU(H264RawPixelsSink* sink, u_int8_t type, size_t size);
    void sinkOnAddedNALU(H264RawPixelsSink* sink, u_int8_t type, size_t size);
  
    boost::mutex frameMapMutex;
    std::map<int64_t, std::vector<AVFrame*> > frameMap;
    std::vector<H264RawPixelsSink*> sinks;
    AVPixelFormat                   format;
    HeapAllocator                   heapAllocator;
    boost::thread                   getNextCubemapThread;
    boost::thread                   getNextFramesThread;
	StereoCubemap*                  oldCubemap;
};