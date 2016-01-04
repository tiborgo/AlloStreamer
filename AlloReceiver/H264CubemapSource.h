#pragma once

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <boost/filesystem/path.hpp>
#include <map>

#include "AlloReceiver.h"
#include "H264NALUSink.hpp"
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
    typedef std::function<void (H264CubemapSource*, int64_t)>                   OnScheduledCubemap;
    
    virtual void setOnReceivedNALU           (const OnReceivedNALU&            callback);
    virtual void setOnReceivedFrame          (const OnReceivedFrame&           callback);
    virtual void setOnDecodedFrame           (const OnDecodedFrame&            callback);
    virtual void setOnColorConvertedFrame    (const OnColorConvertedFrame&     callback);
    virtual void setOnNextCubemap            (const OnNextCubemap&             callback);
    virtual void setOnAddedFrameToCubemap    (const OnAddedFrameToCubemap&     callback);
    virtual void setOnScheduledFrameInCubemap(const OnScheduledFrameInCubemap& callback);
    virtual void setOnScheduledCubemap       (const OnScheduledCubemap&        callback);
    
    H264CubemapSource(std::vector<H264NALUSink*>& sinks,
                      AVPixelFormat               format,
                      bool                        matchStereoPairs,
                      bool                        robustSyncing,
                      size_t                      maxFrameMapSize);

protected:
    OnReceivedNALU            onReceivedNALU;
    OnReceivedFrame           onReceivedFrame;
    OnDecodedFrame            onDecodedFrame;
    OnColorConvertedFrame     onColorConvertedFrame;
    OnNextCubemap             onNextCubemap;
    OnAddedFrameToCubemap     onAddedFrameToCubemap;
    OnScheduledFrameInCubemap onScheduledFrameInCubemap;
    OnScheduledCubemap        onScheduledCubemap;
    
private:
    void getNextFramesLoop();
    void getNextCubemapLoop();
    
    void sinkOnReceivedNALU       (H264NALUSink* sink, u_int8_t type, size_t size);
    void sinkOnReceivedFrame      (H264NALUSink* sink, u_int8_t type, size_t size);
    void sinkOnDecodedFrame       (H264NALUSink* sink, u_int8_t type, size_t size);
    void sinkOnColorConvertedFrame(H264NALUSink* sink, u_int8_t type, size_t size);
  
    boost::mutex                              frameMapMutex;
    boost::condition_variable                 frameMapCondition;
    std::map<int, std::vector<AVFrame*> >     frameMap;
    std::vector<H264NALUSink*>                sinks;
    std::map<H264NALUSink*, int64_t>          sinksFaceMap;
    AVPixelFormat                             format;
    HeapAllocator                             heapAllocator;
    boost::thread                             getNextCubemapThread;
    boost::thread                             getNextFramesThread;
    StereoCubemap*                            oldCubemap;
    int64_t                                   lastFrameSeqNum;
    bool                                      matchStereoPairs;
    bool                                      robustSyncing;
    size_t                                    maxFrameMapSize;
};
