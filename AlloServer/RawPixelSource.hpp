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
                                     int avgBitRate,
									 bool robustSyncing);

	typedef std::function<void(RawPixelSource* self,
		                       uint8_t type,
		                       size_t size)> OnSentNALU;
	typedef std::function<void(RawPixelSource* self)> OnEncodedFrame;

	void setOnSentNALU    (const OnSentNALU&     callback);
	void setOnEncodedFrame(const OnEncodedFrame& callback);

protected:
	RawPixelSource(UsageEnvironment& env,
                   Frame* content,
                   int avgBitRate,
				   bool robustSyncing);
	// called only by createNew(), or by subclass constructors
	virtual ~RawPixelSource();

	OnSentNALU     onSentNALU;
	OnEncodedFrame onEncodedFrame;

private:
	EventTriggerId eventTriggerId;
	static void deliverFrame0(void* clientData);
	static boost::mutex triggerEventMutex;
	static std::vector<RawPixelSource*> sourcesReadyForDelivery;
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

	Frame* content;
	AVCodecContext* codecContext;

	boost::thread frameContentThread;
	boost::thread encodeFrameThread;

	void frameContentLoop();
	void encodeFrameLoop();

	bool destructing;

	int64_t lastFrameTime;

	int_least64_t lastPTS;
	bool robustSyncing;
};
