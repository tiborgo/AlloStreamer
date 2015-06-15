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
    virtual StereoCubemap* getCurrentCubemap();
    
    H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks, int resolution, AVPixelFormat format);
    
private:
    std::vector<H264RawPixelsSink*> sinks;
    std::vector<AVFrame*>           lastFrames;
    SwsContext*                     resizeCtx;
    int                             resolution;
    AVPixelFormat                   format;
    HeapAllocator                   heapAllocator;
};