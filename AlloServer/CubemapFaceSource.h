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

class CubemapFaceSource : public FramedSource
{
public:
	static CubemapFaceSource* createNew(UsageEnvironment& env, CubemapFace* face);

protected:
	CubemapFaceSource(UsageEnvironment& env, CubemapFace* face);
	// called only by createNew(), or by subclass constructors
	virtual ~CubemapFaceSource();

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

	// Stores encoded frames
	concurrent_queue<AVPacket> pktBuffer;
	concurrent_queue<AVPacket> pktPool;

	static unsigned referenceCount; // used to count how many instances of this class currently exist

	CubemapFace* face;
	AVCodecContext* codecContext;

	boost::thread frameFaceThread;
	boost::thread encodeFrameThread;

	void frameFaceLoop();
	void encodeFrameLoop();

	bool destructing;

	int64_t lastFrameTime;
};
