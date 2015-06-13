#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"

class H264CubemapSource : public CubemapSource, private RTSPCubemapSourceClientDelegate
{
public:
    virtual StereoCubemap* getCurrentCubemap();
    
    H264CubemapSource(const char* url, int resolution, AVPixelFormat format);
    
private:
    virtual MediaSink* getSinkForSubsession(RTSPCubemapSourceClient* client, MediaSubsession* subsession);
    
    std::vector<H264RawPixelsSink*> sinks;
    std::vector<AVFrame*>           lastFrames;
    SwsContext*                     resizeCtx;
    int                             resolution;
    AVPixelFormat                   format;
    HeapAllocator                   heapAllocator;
    RTSPCubemapSourceClient*        client;
};