#pragma once

#include <FramedSource.hh>
#include <boost/thread/barrier.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
    #include <libavformat/avformat.h>
    #include <x264.h>
}

#include "AlloShared/concurrent_queue.h"
#include "AlloShared/Cubemap.hpp"

class RawPixelSource : public FramedSource
{
public:
	static RawPixelSource* createNew(UsageEnvironment& env,
                                     Frame* content,
                                     int avgBitRate);

protected:
	RawPixelSource(UsageEnvironment& env,
                   Frame* content,
                   int avgBitRate);
	// called only by createNew(), or by subclass constructors
	virtual ~RawPixelSource();

private:
	EventTriggerId eventTriggerId;
	static void deliverFrame0(void* clientData);
	void deliverFrame();

	// redefined virtual functions:
	virtual void doGetNextFrame();
	//virtual void doStopGettingFrames(); // optional

	int x2yuv(AVFrame *xFrame, AVFrame *yuvFrame, AVCodecContext *c);
	SwsContext *img_convert_ctx;

	// Stores unencoded frames
	concurrent_queue<AVFrame*> frameBuffer;
	// Here unused frames are stored. Included so that we can allocate all the frames at startup
	// and reuse them during runtime
	concurrent_queue<AVFrame*> framePool;

	concurrent_queue<AVFrame*> convertedFrameBuffer;
	concurrent_queue<AVFrame*> convertedFramePool;

	// Stores encoded frames
	concurrent_queue<AVPacket> pktBuffer;
	concurrent_queue<AVPacket> pktPool;

	static unsigned referenceCount; // used to count how many instances of this class currently exist

	Frame* content;
	AVCodecContext* codecContext;

	boost::thread frameContentThread;
	boost::thread convertFormatThread;
	boost::thread encodeFrameThread;

	void frameContentLoop();
	void convertFormatLoop();
	void encodeFrameLoop();

	bool destructing;

	int64_t lastFrameTime;
};
