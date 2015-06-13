#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"

class RTSPCubemapSourceClient;

class RTSPCubemapSourceClientDelegate
{
public:
    virtual MediaSink* getSinkForSubsession(RTSPCubemapSourceClient* client, MediaSubsession* subsession) = 0;
};

class RTSPCubemapSourceClient : public RTSPClient
{
public:
    void connect();
    static RTSPCubemapSourceClient* create();
    
    static RTSPCubemapSourceClient* createNew(char const* rtspURL,
                                              int verbosityLevel = 0,
                                              char const* applicationName = NULL,
                                              portNumBits tunnelOverHTTPPortNum = 0,
                                              int socketNumToServer = -1);
    
    RTSPCubemapSourceClientDelegate* delegate;
    
protected:
    RTSPCubemapSourceClient(UsageEnvironment& env,
                            char const* rtspURL,
                            int verbosityLevel,
                            char const* applicationName,
                            portNumBits tunnelOverHTTPPortNum,
                            int socketNumToServer);
    
    static void continueAfterDESCRIBE2 (RTSPClient* self,
                                        int resultCode,
                                        char* resultString);
    static void continueAfterPLAY      (RTSPClient* self,
                                        int resultCode,
                                        char* resultString);
    static void continueAfterSETUP     (RTSPClient* self,
                                        int resultCode,
                                        char* resultString);
    static void continueAfterDESCRIBE  (RTSPClient* self,
                                        int resultCode,
                                        char* resultString);
    static void continueAfterOPTIONS   (RTSPClient* self,
                                        int resultCode,
                                        char* resultString);
    
    static void subsessionByeHandler   (void* self);
    static void subsessionAfterPlaying (void* self);
    static void checkForPacketArrival  (void* self);
    
    void networkLoop            ();
    void shutdown               (int exitCode = 1);
    
    void createOutputFiles      (char const* periodicFilenameSuffix);
    void setupStreams           ();
    
private:
    boost::thread networkThread;
};

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