#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"

class H264CubemapSource : public CubemapSource
{
public:
    virtual StereoCubemap* tryGetNextCubemap(int desiredResolution, AVPixelFormat desiredFormat);
    virtual StereoCubemap* tryGetNextCubemap();
    
    H264CubemapSource(const char* url);
    
private:
    static std::vector<H264RawPixelsSink*> sinks;
    static std::vector<AVFrame*> lastFrames;
    
    static void shutdown(int exitCode = 1);
    static void subsessionAfterPlaying(void* clientData);
    static void checkForPacketArrival(void* self);
    static void continueAfterDESCRIBE2(RTSPClient*, int resultCode, char* resultString);
    static void continueAfterPLAY(RTSPClient*, int resultCode, char* resultString);
    static void subsessionByeHandler(void* clientData);
    static void createOutputFiles(char const* periodicFilenameSuffix);
    static void setupStreams();
    static void continueAfterSETUP(RTSPClient*, int resultCode, char* resultString);
    static void continueAfterDESCRIBE(RTSPClient*, int resultCode, char* resultString);
    static void continueAfterOPTIONS(RTSPClient*, int resultCode, char* resultString);
    static void live555Loop(const char* progName, const char* url);
    
    virtual StereoCubemap* tryGetNextCubemapInternal(int desiredResolution, AVPixelFormat desiredFormat, bool resize);
    
    SwsContext* resizeCtx;
};