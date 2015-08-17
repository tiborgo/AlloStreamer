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
    static RTSPCubemapSourceClient* create();
    
    static RTSPCubemapSourceClient* createNew(char const* rtspURL,
                                              unsigned int sinkBufferSize,
                                              int verbosityLevel = 0,
                                              char const* applicationName = NULL,
                                              portNumBits tunnelOverHTTPPortNum = 0,
                                              int socketNumToServer = -1);
    
    std::function<std::vector<MediaSink*> (RTSPCubemapSourceClient*, std::vector<MediaSubsession*>&)> onGetSinksForSubsessions;
    std::function<void (RTSPCubemapSourceClient*)> onDidIdentifyStreams;
    
protected:
    RTSPCubemapSourceClient(UsageEnvironment& env,
                            char const* rtspURL,
                            unsigned int sinkBufferSize,
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
    MediaSession* session;
    MediaSubsession* subsession;
    unsigned int sinkBufferSize;
};