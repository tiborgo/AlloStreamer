#pragma once

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/time.h>
	#include <libswscale/swscale.h>
	#include <x264.h>
}
#include <MediaSink.hh>
#include <MediaSession.hh>
#include <boost/thread.hpp>

#include "AlloServer/concurrent_queue.h"
#include "AlloShared/CubemapFace.h"

class H264RawPixelsSink: public MediaSink
{
public:
	static H264RawPixelsSink* createNew(UsageEnvironment& env,
		unsigned int bufferSize);

	Frame* getNextFrame();

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

	void decodeFrameLoop();
};

