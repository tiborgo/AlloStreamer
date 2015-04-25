
#include "CubemapFaceSource.h"
#include "AlloShared/Signals.h"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include "config.h"
//#include "stdafx.h"
#include <stdint.h>
#include <time.h>
#include <boost/thread.hpp>
#include <boost/interprocess/containers/deque.hpp>

int CubemapFaceSource::x2yuv(AVFrame *xFrame, AVFrame *yuvFrame, AVCodecContext *c)
{
	char *err = NULL;
	if (img_convert_ctx == NULL)
	{
		// MUST BE IMPLMENTED
		int w = xFrame->width;
		int h = xFrame->width;
		img_convert_ctx = sws_getContext(w, h, (AVPixelFormat)xFrame->format, w, h,
			c->pix_fmt, SWS_BICUBIC,
			NULL, NULL, NULL);
		if (img_convert_ctx == NULL)
		{
			sprintf(err, "Cannot initialize the conversion context!\n");
			return -1;
		}
	}
	if (xFrame->linesize[0] > 0)
	{
		xFrame->data[0] += xFrame->linesize[0] * (c->height - 1);

		xFrame->linesize[0] = -xFrame->linesize[0];
	}
	return sws_scale(img_convert_ctx, xFrame->data,
		xFrame->linesize, 0, c->height,
		yuvFrame->data, yuvFrame->linesize);
}

CubemapFaceSource* CubemapFaceSource::createNew(UsageEnvironment& env, CubemapFace* face)
{
	return new CubemapFaceSource(env, face);
}

unsigned CubemapFaceSource::referenceCount = 0;

struct timeval prevtime;

CubemapFaceSource::CubemapFaceSource(UsageEnvironment& env, CubemapFace* face)
: FramedSource(env), face(face), /*encodeBarrier(2),*/ destructing(false),
img_convert_ctx(NULL)
{

	gettimeofday(&prevtime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
	if (referenceCount == 0)
	{
		// Any global initialization of the device would be done here:
		//%%% TO BE WRITTEN %%%
	}

	// Any instance-specific initialization of the device would be done here:

	++referenceCount;
	myfile = fopen("C:/Users/Tibor/Desktop/Logs/deviceglxgears.log", "w");

	// initialize frame pool
	for (int i = 0; i < 10; i++)
	{
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = face->format;
		frame->width = face->width;
		frame->height = face->height;

		/* the image can be allocated by any means and av_image_alloc() is
		* just the most convenient way if av_malloc() is to be used */
		if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
			face->format, 32) < 0)
		{
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			exit(1);
		}

		framePool.push(frame);
	}

	// Initialize codec and encoder
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	codecContext = avcodec_alloc_context3(codec);

	if (!codecContext)
	{
		fprintf(stderr, "could not allocate video codec context\n");
		exit(1);
	}

	/* put sample parameters */
	codecContext->bit_rate = bit_rate;
	/* resolution must be a multiple of two */
	codecContext->width = face->width;
	codecContext->height = face->height;
	/* frames per second */
	codecContext->time_base = { 1, FPS };
	codecContext->gop_size = 20; /* emit one intra frame every ten frames */
	codecContext->max_b_frames = 0;
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	av_opt_set(codecContext->priv_data, "preset", preset_val, 0);
	av_opt_set(codecContext->priv_data, "tune", tune_val, 0);

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	// We arrange here for our "deliverFrame" member function to be called
	// whenever the next frame of data becomes available from the device.
	//
	// If the device can be accessed as a readable socket, then one easy way to do this is using a call to
	//     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
	// (See examples of this call in the "liveMedia" directory.)
	//
	// If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
	// Create an 'event trigger' for this device (if it hasn't already been done):
	eventTriggerId = envir().taskScheduler().createEventTrigger(&deliverFrame0);

	frameFaceThread = boost::thread(boost::bind(&CubemapFaceSource::frameFaceLoop, this));

	encodeFrameThread = boost::thread(boost::bind(&CubemapFaceSource::encodeFrameLoop, this));

	lastFrameTime = av_gettime();
}

CubemapFaceSource::~CubemapFaceSource()
{
	// Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
	std::cout << this << ": deconstructing..." << std::endl;

	this->destructing = true;
	frameBuffer.close();
	pktBuffer.close();

	frameFaceThread.join();
	encodeFrameThread.join();

	--referenceCount;
	if (referenceCount == 0)
	{
		// Any global 'destruction' (i.e., resetting) of the device would be done here:


	}

	// Reclaim our 'event trigger'
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	eventTriggerId = 0;
	fclose(myfile);

	std::cout << this << ": deconstructed" << std::endl;
}

void CubemapFaceSource::frameFaceLoop()
{

	while (!destructing)
	{
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(face->mutex);

		AVFrame* frame;

		// Only frame face if spare faces are available
		// Otherwise skip this frame
		if (framePool.try_pop(frame))
		{
			// Fill frame
			avpicture_fill((AVPicture*)frame,
				(uint8_t*)this->face->pixels.get(),
				face->format,
				this->face->width,
				this->face->height);

			// Make frame available to the encoder
			frameBuffer.push(frame);
		}

		// Wait for new frame
		face->newPixelsCondition.wait(lock);
	}
}

void CubemapFaceSource::doGetNextFrame()
{
	// This function is called (by our 'downstream' object) when it asks for new data.

	// Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
	if (0 /* the source stops being readable */ /*%%% TO BE WRITTEN %%%*/)
	{
		handleClosure(this);
		return;
	}

	// If a new frame of data is immediately available to be delivered, then do this now:
	if (!pktBuffer.empty())
	{
		deliverFrame();
	}

	// No new data is immediately available to be delivered.  We don't do anything more here.
	// Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.

}

void CubemapFaceSource::deliverFrame0(void* clientData)
{
	//std::cout << "deliver frame: " << ((CubemapFaceSource*)clientData)->face->index << std::endl;
	((CubemapFaceSource*)clientData)->deliverFrame();
}

void CubemapFaceSource::encodeFrameLoop()
{


	while (!this->destructing)
	{
		// Pop frame ptr from buffer
		AVFrame* xFrame;
		AVFrame* yuv420pFrame;
		AVPacket pkt;




		if (!frameBuffer.wait_and_pop(xFrame))
		{
			// queue did close
			return;
		}



		yuv420pFrame = av_frame_alloc();
		if (!yuv420pFrame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			return;
		}
		yuv420pFrame->format = AV_PIX_FMT_YUV420P;
		yuv420pFrame->width = xFrame->width;
		yuv420pFrame->height = xFrame->height;



		/* the image can be allocated by any means and av_image_alloc() is
		* just the most convenient way if av_malloc() is to be used */
		if (av_image_alloc(yuv420pFrame->data, yuv420pFrame->linesize, yuv420pFrame->width, yuv420pFrame->height,
			AV_PIX_FMT_YUV420P, 32) < 0)
		{
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			return;
		}

		x2yuv(xFrame, yuv420pFrame, codecContext);

		av_init_packet(&pkt);
		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;
		int got_output = 0;

		//mutex.lock();
		int ret = avcodec_encode_video2(codecContext, &pkt, yuv420pFrame, &got_output);
		//mutex.unlock();

		fprintf(myfile, "packet size: %i \n", pkt.size);
		fflush(myfile);

		if (ret < 0)
		{
			fprintf(stderr, "Error encoding frame\n");
			return;
		}

		framePool.push(xFrame);
		pktBuffer.push(pkt);

		//std::cout << this << ": encoded frame" << std::endl;

		if (!this->destructing && isCurrentlyAwaitingData())
		{
			envir().taskScheduler().triggerEvent(eventTriggerId, this);
		}

		av_freep(&yuv420pFrame->data[0]);
		av_frame_free(&yuv420pFrame);
	}
}

void CubemapFaceSource::deliverFrame()
{
	// This function is called when new frame data is available from the device.
	// We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
	// 'in' parameters (these should *not* be modified by this function):
	//     fTo: The frame data is copied to this address.
	//         (Note that the variable "fTo" is *not* modified.  Instead,
	//          the frame data is copied to the address pointed to by "fTo".)
	//     fMaxSize: This is the maximum number of bytes that can be copied
	//         (If the actual frame is larger than this, then it should
	//          be truncated, and "fNumTruncatedBytes" set accordingly.)
	// 'out' parameters (these are modified by this function):
	//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
	//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
	//         bigger than "fMaxSize", in which case it's set to the number of bytes
	//         that have been omitted.
	//     fPresentationTime: Should be set to the frame's presentation time
	//         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
	//         by calling "gettimeofday()".
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
	//         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
	//         to set this variable, because - in this case - data will never arrive 'early'.
	// Note the code below.



	if (!isCurrentlyAwaitingData())
	{
		return; // we're not ready for the data yet
	}


	int64_t thisTime = av_gettime();

	fprintf(myfile, "fMaxSize at beginning of function: %i \n", fMaxSize);
	fflush(myfile);

	// set the duration of this frame since we have variable frame rate
	// %% Time has to be fixed
	this->fDurationInMicroseconds = 1000000 / 70;// thisTime - lastFrameTime;

	gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.

	std::cout << this << ": pktBuffer size: " << pktBuffer.size() << std::endl;

	AVPacket pkt;
	if (!pktBuffer.wait_and_pop(pkt))
	{
		// queue did close
		return;
	}

	u_int8_t* newFrameDataStart = (u_int8_t*)pkt.data;
	unsigned newFrameSize = pkt.size;

	// Deliver the data here:
	if (newFrameSize > fMaxSize)
	{
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = newFrameSize - fMaxSize;
		fprintf(myfile, "frameSize %i larger than maxSize %i\n", pkt.size, fMaxSize);
		fflush(myfile);
	}
	else
	{
		fFrameSize = newFrameSize;
		/*rest =*/ fNumTruncatedBytes = 0;
		//
	}

	memmove(fTo, newFrameDataStart, fFrameSize);


	av_free_packet(&pkt);

	if (fNumTruncatedBytes > 0)
	{
		std::cout << this << ": truncated " << fNumTruncatedBytes << " bytes" << std::endl;
	}

	// Tell live555 that a new frame is available
	FramedSource::afterGetting(this);

	lastFrameTime = thisTime;
}
