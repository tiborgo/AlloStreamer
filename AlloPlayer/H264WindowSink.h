#pragma once

//#include <windows.h>

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

#include "AlloServer/concurrent_queue.h"

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

	static void afterGettingFrame(void*clientData,
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned durationInMicroseconds);

	static LRESULT WINAPI MsgProc0(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	unsigned int bufferSize;
	unsigned char* buffer;
	//AVFrame* frame;
	AVCodecContext* codecContext;
	MediaSubsession& subSession;
	HWND hWnd;
	concurrent_queue<AVFrame*> frameBuffer;
	concurrent_queue<AVFrame*> framePool;

	void windowLoop();

	SwsContext *img_convert_ctx;

	int x2y(AVFrame *srcFrame, AVFrame *dstFrame, AVCodecContext *c);
};

