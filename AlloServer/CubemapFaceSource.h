#pragma once

#include "FramedSource.hh"
#include "shared.h"
#include <boost/thread/barrier.hpp>
#include <boost/thread/synchronized_value.hpp>
#include "concurrent_queue.h"
#include "AlloShared/CubemapFace.h"
#include <boost/thread/condition.hpp>

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
	//boost::synchronized_value<concurrent_queue<AVPacket>> pktBuffer;
	concurrent_queue<AVPacket> pktBuffer;

	static unsigned referenceCount; // used to count how many instances of this class currently exist

	CubemapFace* face;
	AVCodecContext* codecContext;

	FILE * myfile;

	boost::thread frameFaceThread;
	boost::thread encodeFrameThread;

	void frameFaceLoop();
	void encodeFrameLoop();

	bool destructing;

	int64_t lastFrameTime;
};
