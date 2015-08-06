#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"
#include "RTSPCubemapSource.hpp"

class ALLORECEIVER_API H264CubemapSource : public RTSPCubemapSource
{
public:
    virtual void setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>&          callback);
    virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int face, u_int8_t type)>& callback);
    virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int face, u_int8_t type)>& callback);
    
    H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks, int resolution, AVPixelFormat format);

protected:
    std::function<void (CubemapSource*, StereoCubemap*)> onNextCubemap;
    std::function<void (CubemapSource*, int face, u_int8_t type)> onDroppedNALU;
    std::function<void (CubemapSource*, int face, u_int8_t type)> onAddedNALU;
    
private:
    void getNextCubemapLoop();
    void sinkOnDroppedNALU(H264RawPixelsSink* sink, u_int8_t type);
    void sinkOnAddedNALU(H264RawPixelsSink* sink, u_int8_t type);
    
    std::vector<H264RawPixelsSink*> sinks;
    int                             resolution;
    AVPixelFormat                   format;
    HeapAllocator                   heapAllocator;
    boost::thread                   getNextCubemapThread;
    
};