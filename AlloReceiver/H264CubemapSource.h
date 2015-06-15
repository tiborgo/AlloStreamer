#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"
#include "RTSPCubemapSource.hpp"

class H264CubemapSource : public RTSPCubemapSource
{
public:
    virtual void setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>& onNextCubemap);
    
    H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks, int resolution, AVPixelFormat format);

protected:
    std::function<void (CubemapSource*, StereoCubemap*)> onNextCubemap;
    
private:
    void getNextCubemapLoop();
    
    std::vector<H264RawPixelsSink*> sinks;
    SwsContext*                     resizeCtx;
    int                             resolution;
    AVPixelFormat                   format;
    HeapAllocator                   heapAllocator;
    boost::thread                   getNextCubemapThread;
};