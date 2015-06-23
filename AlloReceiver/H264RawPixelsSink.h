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
}
#include <MediaSink.hh>
#include <MediaSession.hh>
#include <boost/thread.hpp>

#include "AlloShared/concurrent_queue.h"
#include "AlloShared/Cubemap.hpp"

class H264RawPixelsSink: public MediaSink
{
public:
	static H264RawPixelsSink* createNew(UsageEnvironment& env,
		unsigned int bufferSize);

    AVFrame* getNextFrame();
    
    void setOnDroppedNALU(std::function<void (H264RawPixelsSink*, u_int8_t type)>& callback);
    void setOnAddedNALU(std::function<void (H264RawPixelsSink*, u_int8_t type)>& callback);

protected:
	H264RawPixelsSink(UsageEnvironment& env,
		unsigned int bufferSize);

	virtual void afterGettingFrame(unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime);

	virtual Boolean continuePlaying();

	static void afterGettingFrame(void*clientData,
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned durationInMicroseconds);
    
    std::function<void (H264RawPixelsSink*, u_int8_t type)> onDroppedNALU;
    std::function<void (H264RawPixelsSink*, u_int8_t type)> onAddedNALU;

private:
	unsigned int bufferSize;
	unsigned char* buffer;
	AVCodecContext* codecContext;
	concurrent_queue<AVFrame*> frameBuffer;
	concurrent_queue<AVFrame*> framePool;
	concurrent_queue<AVPacket*> pktBuffer;
    concurrent_queue<AVPacket*> pktPool;

	SwsContext *img_convert_ctx;

	boost::thread decodeFrameThread;
    AVPacket* lastIFramePkt;
    bool gotFirstIFrame;
    
	void decodeFrameLoop();

	int counter;
	long sumRelativePresentationTimeMicroSec;
	long maxRelativePresentationTimeMicroSec;
    
    void packageData(AVPacket* pkt, unsigned int frameSize, timeval presentationTime);
};

