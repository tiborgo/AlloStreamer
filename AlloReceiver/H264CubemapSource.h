#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>
#include <map>

#include "AlloReceiver.h"
#include "H264RawPixelsSink.h"
#include "RTSPCubemapSourceClient.hpp"

class ALLORECEIVER_API H264CubemapSource : public CubemapSource
{
public:
    typedef std::function<void (H264CubemapSource*, u_int8_t, size_t, int)>     OnReceivedNALU;
    typedef std::function<void (H264CubemapSource*, u_int8_t, size_t, int)>     OnReceivedFrame;
    typedef std::function<void (H264CubemapSource*, u_int8_t, size_t, int)>     OnDecodedFrame;
    typedef std::function<void (H264CubemapSource*, u_int8_t, size_t, int)>     OnColorConvertedFrame;
    typedef std::function<void (H264CubemapSource*, int)>                       OnAddedFrameToCubemap;
    typedef std::function<void (H264CubemapSource*, int)>                       OnScheduledFrameInCubemap;
    
    virtual void setOnReceivedNALU           (const OnReceivedNALU&            callback);
    virtual void setOnReceivedFrame          (const OnReceivedFrame&           callback);
    virtual void setOnDecodedFrame           (const OnDecodedFrame&            callback);
    virtual void setOnColorConvertedFrame    (const OnColorConvertedFrame&     callback);
    virtual void setOnNextCubemap            (const OnNextCubemap&             callback);
    virtual void setOnAddedFrameToCubemap    (const OnAddedFrameToCubemap&     callback);
    virtual void setOnScheduledFrameInCubemap(const OnScheduledFrameInCubemap& callback);
    
    H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks,
                      AVPixelFormat                    format,
                      bool                             matchStereoPairs);

protected:
    OnReceivedNALU            onReceivedNALU;
    OnReceivedFrame           onReceivedFrame;
    OnDecodedFrame            onDecodedFrame;
    OnColorConvertedFrame     onColorConvertedFrame;
    OnNextCubemap             onNextCubemap;
    OnAddedFrameToCubemap     onAddedFrameToCubemap;
    OnScheduledFrameInCubemap onScheduledFrameInCubemap;
    
private:
    void getNextFramesLoop();
    void getNextCubemapLoop();
    
    void sinkOnReceivedNALU       (H264RawPixelsSink* sink, u_int8_t type, size_t size);
    void sinkOnReceivedFrame      (H264RawPixelsSink* sink, u_int8_t type, size_t size);
    void sinkOnDecodedFrame       (H264RawPixelsSink* sink, u_int8_t type, size_t size);
    void sinkOnColorConvertedFrame(H264RawPixelsSink* sink, u_int8_t type, size_t size);
  
    boost::mutex                              frameMapMutex;
    boost::condition_variable                 frameMapCondition;
    std::map<int, std::vector<AVFrame*> >     frameMap;
    std::vector<H264RawPixelsSink*>           sinks;
    std::map<H264RawPixelsSink*, int>         sinksFaceMap;
    AVPixelFormat                             format;
    HeapAllocator                             heapAllocator;
    boost::thread                             getNextCubemapThread;
    boost::thread                             getNextFramesThread;
    StereoCubemap*                            oldCubemap;
    int                                       lastFrameSeqNum;
    bool                                      matchStereoPairs;
};
