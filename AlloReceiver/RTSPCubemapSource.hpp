#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"

class ALLORECEIVER_API RTSPCubemapSource : public CubemapSource
{
public:
    static RTSPCubemapSource* create(const char* url,
                                     int resolution,
                                     AVPixelFormat format,
                                     const char* interfaceAddress = "0.0.0.0");
    
private:
    std::vector<MediaSink*> onGetSinksForSubsessions(RTSPCubemapSourceClient* client,
                                                     std::vector<MediaSubsession*>& subsessions);
    void onDidIdentifyStreams(RTSPCubemapSourceClient* client);
    
    int                             counter;
    std::vector<H264RawPixelsSink*> sinks;
    RTSPCubemapSourceClient*        client;
};
