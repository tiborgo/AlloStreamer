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
    // Blocks until RTSP description has been read and all stream are connected
    static RTSPCubemapSource* createFromRTSP(const char*   url,
		                                     unsigned long bufferSize,
                                             AVPixelFormat format,
                                             const char*   interfaceAddress = "0.0.0.0");
    
    virtual void setOnNextCubemap(std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)>& callback);
    virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback);
    virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback);
    
    const RTSPCubemapSourceClient* getClient();
    
protected:
    RTSPCubemapSource(RTSPCubemapSourceClient* client,
                      CubemapSource*           cubemapSource);
    
    std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)> onNextCubemap;
    std::function<void (CubemapSource*, int, u_int8_t, size_t)>    onDroppedNALU;
    std::function<void (CubemapSource*, int, u_int8_t, size_t)>    onAddedNALU;
    
private:
    std::vector<MediaSink*> onGetSinksForSubsessions(RTSPCubemapSourceClient*       client,
                                                     std::vector<MediaSubsession*>& subsessions);
    
    RTSPCubemapSourceClient*        client;
    CubemapSource*                  cubemapSource;
};
