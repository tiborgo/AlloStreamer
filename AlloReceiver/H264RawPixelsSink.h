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

#include "AlloShared/ConcurrentQueue.hpp"
#include "AlloShared/Cubemap.hpp"

class ALLORECEIVER_API H264RawPixelsSink : public MediaSink
{
public:
	static H264RawPixelsSink* createNew(UsageEnvironment& env,
                                        unsigned long     bufferSize,
                                        AVPixelFormat     format,
                                        MediaSubsession*  subsession,
                                        bool              robustSyncing);

	AVFrame* getNextFrame();
    void returnFrame(AVFrame* usedFrame);
    
    typedef std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> OnReceivedNALU;
    typedef std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> OnReceivedFrame;
    typedef std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> OnDecodedFrame;
    typedef std::function<void (H264RawPixelsSink*, u_int8_t, size_t)> OnColorConvertedFrame;
    
    void setOnReceivedNALU       (const OnReceivedNALU&        callback);
    void setOnReceivedFrame      (const OnReceivedFrame&       callback);
    void setOnDecodedFrame       (const OnDecodedFrame&        callback);
    void setOnColorConvertedFrame(const OnColorConvertedFrame& callback);
	
protected:
	H264RawPixelsSink(UsageEnvironment& env,
                      unsigned int      bufferSize,
                      AVPixelFormat     format,
                      MediaSubsession*  subsession,
                      bool              robustSyncing);

	virtual void afterGettingFrame(unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime);

	virtual Boolean continuePlaying();

	static void afterGettingFrame(void*clientData, 
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned durationInMicroseconds);
    
    OnReceivedNALU        onReceivedNALU;
    OnReceivedFrame       onReceivedFrame;
    OnDecodedFrame        onDecodedFrame;
    OnColorConvertedFrame onColorConvertedFrame;

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
    ConcurrentQueue<NALU*> naluPool;
    ConcurrentQueue<NALU*> naluBuffer;
    ConcurrentQueue<AVPacket*> pktBuffer;
    ConcurrentQueue<AVPacket*> pktPool;
	ConcurrentQueue<AVFrame*> frameBuffer;
	ConcurrentQueue<AVFrame*> framePool;
    ConcurrentQueue<AVFrame*> convertedFrameBuffer;
    ConcurrentQueue<AVFrame*> convertedFramePool;
    
    AVPacket* currentPkt;
    int64_t pts;
    int64_t lastPTS;
    
    bool robustSyncing;
    
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

