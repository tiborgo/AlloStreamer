#pragma once

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/frame.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/time.h>
	#include <libswscale/swscale.h>
	#include <x264.h>
    #include <libavformat/avformat.h>
}
#include <MediaSink.hh>
#include <MediaSession.hh>
#include <boost/thread.hpp>

#include "AlloReceiver.h"

#include "AlloShared/concurrent_queue.h"
#include "AlloShared/Cubemap.hpp"

class ALLORECEIVER_API H264RawPixelsSink : public MediaSink
{
public:
	static H264RawPixelsSink* createNew(UsageEnvironment& env,
                                        unsigned long bufferSize,
                                        AVPixelFormat format,
                                        MediaSubsession* subsession);

	AVFrame* getNextFrame();
    void returnFrame(AVFrame* usedFrame);
    
    void setOnDroppedNALU(std::function<void (H264RawPixelsSink*, u_int8_t, size_t)>& callback);
    void setOnAddedNALU(std::function<void (H264RawPixelsSink*, u_int8_t, size_t)>& callback);
	
protected:
	H264RawPixelsSink(UsageEnvironment& env,
                      unsigned int bufferSize,
                      AVPixelFormat format,
                      MediaSubsession* subsession);

	virtual void afterGettingFrame(unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime);

	virtual Boolean continuePlaying();

	static void afterGettingFrame(void*clientData, 
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned durationInMicroseconds);
    
    std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> onDroppedNALU;
    std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> onAddedNALU;

private:
    struct NALU
    {
        unsigned char* buffer;
        size_t size;
        int64_t pts;
    };
    
	unsigned long bufferSize;
	unsigned char* buffer;
	AVCodecContext* codecContext;
    concurrent_queue<NALU*> naluPool;
    concurrent_queue<NALU*> naluBuffer;
    concurrent_queue<AVPacket*> pktBuffer;
    concurrent_queue<AVPacket*> pktPool;
	concurrent_queue<AVFrame*> frameBuffer;
	concurrent_queue<AVFrame*> framePool;
    concurrent_queue<AVFrame*> resizedFrameBuffer;
    concurrent_queue<AVFrame*> resizedFramePool;

	SwsContext* imageConvertCtx;
    
    std::queue<AVPacket*> priorityPackages; // SPS, PPS, IDR-slice NAL
    bool receivedFirstPriorityPackages; // first sequence of SPS, PPS and IDR-slice NALUs has been received
    
    AVPixelFormat format;
    
    void packageNALUsLoop();
	void decodeFrameLoop();
    void convertFrameLoop();
    boost::thread packageNALUsThread;
    boost::thread decodeFrameThread;
    boost::thread convertFrameThread;

	int counter;
	long sumRelativePresentationTimeMicroSec;
	long maxRelativePresentationTimeMicroSec;
    
    MediaSubsession* subsession;
    int lastTotal;
    
    void packageData(AVPacket* pkt, unsigned int frameSize, timeval presentationTime);
};

