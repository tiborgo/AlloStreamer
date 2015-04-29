#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <x264.h>
}
#include <MediaSink.hh>
#include <MediaSession.hh>

class H264WindowSink : public MediaSink
{
public:
	static H264WindowSink* createNew(UsageEnvironment& env,
		unsigned int bufferSize,
		MediaSubsession& subSession);

protected:
	H264WindowSink(UsageEnvironment& env,
		unsigned int bufferSize,
		MediaSubsession& subSession);

	virtual void afterGettingFrame(unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime);

	virtual Boolean continuePlaying();

	static void H264WindowSink::afterGettingFrame(void*clientData,
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned durationInMicroseconds);

private:
	unsigned int bufferSize;
	unsigned char* buffer;
	AVFrame* frame;
	AVCodecContext* codecContext;
	MediaSubsession& subSession;
};

