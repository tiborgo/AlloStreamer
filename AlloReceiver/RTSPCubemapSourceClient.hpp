#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/thread.hpp>

#include "AlloReceiver.h"

class ALLORECEIVER_API RTSPCubemapSourceClient : public RTSPClient
{
public:
    void connect();

    static RTSPCubemapSourceClient* create(char const* rtspURL,
                                           unsigned int sinkBufferSize,
                                           AVPixelFormat format,
                                           const char* interface = "0.0.0.0",
                                           int verbosityLevel = 0,
                                           char const* applicationName = NULL,
                                           portNumBits tunnelOverHTTPPortNum = 0,
                                           int socketNumToServer = -1);
    
    void setOnDidConnect(std::function<void (RTSPCubemapSourceClient*, CubemapSource*)>& onDidConnect);
    
protected:
    RTSPCubemapSourceClient(UsageEnvironment& env,
                            char const* rtspURL,
                            unsigned int sinkBufferSize,
                            AVPixelFormat format,
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
    
    std::function<void (RTSPCubemapSourceClient*, CubemapSource*)> onDidConnect;
    
private:
	std::vector<BasicUsageEnvironment*> envs;
	std::vector<boost::shared_ptr<boost::thread>> sessionThreads;
    boost::thread networkThread;
	std::vector<MediaSubsession*> subsessions;
    MediaSubsession* subsession;
    unsigned int sinkBufferSize;
    AVPixelFormat format;
};