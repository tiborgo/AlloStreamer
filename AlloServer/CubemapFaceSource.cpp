#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <stdint.h>
#include <time.h>
#include <boost/thread.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "config.h"
#include "CubemapFaceSource.h"

namespace bc = boost::chrono;

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

CubemapFaceSource* CubemapFaceSource::createNew(UsageEnvironment& env,
	                                            CubemapFace* face,
												int avgBitRate)
{
	return new CubemapFaceSource(env, face, avgBitRate);
}

unsigned CubemapFaceSource::referenceCount = 0;

struct timeval prevtime;

CubemapFaceSource::CubemapFaceSource(UsageEnvironment& env,
	                                 CubemapFace* face,
									 int avgBitRate)
	:
	FramedSource(env), img_convert_ctx(NULL), face(face), /*encodeBarrier(2),*/ destructing(false)
{

	gettimeofday(&prevtime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
	if (referenceCount == 0)
	{
		// Any global initialization of the device would be done here:
		//%%% TO BE WRITTEN %%%
	}

	// Any instance-specific initialization of the device would be done here:

	++referenceCount;
	//myfile = fopen("/Users/tiborgoldschwendt/Desktop/Logs/deviceglxgears.log", "w");

	// initialize frame pool
	for (int i = 0; i < 1; i++)
	{
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = face->getFormat();
		frame->width = face->getWidth();
		frame->height = face->getHeight();

		/* the image can be allocated by any means and av_image_alloc() is
		* just the most convenient way if av_malloc() is to be used */
		if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
			face->getFormat(), 32) < 0)
		{
			fprintf(stderr, "Could not allocate raw picture buffer\n");
			abort();
		}

		framePool.push(frame);
	}

	for (int i = 0; i < 1; i++)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		pktPool.push(pkt);
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
	codecContext->bit_rate = avgBitRate;
	/* resolution must be a multiple of two */
	codecContext->width = face->getWidth();
	codecContext->height = face->getHeight();
	/* frames per second */
	codecContext->time_base = av_make_q(1, FPS);
	codecContext->gop_size = 20; /* emit one intra frame every ten frames */
	codecContext->max_b_frames = 0;
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	//codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

	av_opt_set(codecContext->priv_data, "preset", PRESET_VAL, 0);
	av_opt_set(codecContext->priv_data, "tune", TUNE_VAL, 0);

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

	//std::cout << this << ": eventTriggerId: " << eventTriggerId  << std::endl;

	frameFaceThread = boost::thread(boost::bind(&CubemapFaceSource::frameFaceLoop, this));

	encodeFrameThread = boost::thread(boost::bind(&CubemapFaceSource::encodeFrameLoop, this));

	lastFrameTime = av_gettime();
}

CubemapFaceSource::~CubemapFaceSource()
{
	// Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
	//std::cout << this << ": deconstructing..." << std::endl;

	this->destructing = true;
	frameBuffer.close();
	pktBuffer.close();
	framePool.close();
	pktPool.close();

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
	//fclose(myfile);

	//std::cout << this << ": deconstructed" << std::endl;
}

void CubemapFaceSource::frameFaceLoop()
{

	while (!destructing)
	{
        while (!face->getMutex().timed_lock(boost::get_system_time() + boost::posix_time::milliseconds(100)))
        {
            if (destructing)
            {
                return;
            }
        }
        
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(face->getMutex(),
                                                                                       boost::interprocess::accept_ownership);

		AVFrame* frame;
		if (!framePool.wait_and_pop(frame))
		{
            return;
        }
        
        // barrier1  // barrier1
                     // barrier2
        
        // Fill frame
        avpicture_fill((AVPicture*)frame,
            (uint8_t*)face->getPixels(),
            face->getFormat(),
            face->getWidth(),
            face->getHeight());

		std::cout << "framed face" << std::endl;
        
        // barrier2

        // Set the actual presentation time
        // It is in the past probably but we will try our best
        AVRational microSecBase = { 1, 1000000 };
        bc::microseconds presentationTimeSinceEpochMicroSec =
            bc::duration_cast<bc::microseconds>(face->getPresentationTime().time_since_epoch());
        
        
//        const time_t time = bc::system_clock::to_time_t(face->getPresentationTime());
//
//        // Maybe the put_time will be implemented later?
//        struct tm* tm = localtime(&time);
//        char timeStr[128];
//        strftime (timeStr, sizeof(timeStr), "%c",tm);
        
        //std::cout << this << " presentation time: " << timeStr << std::endl;
        //std::cout << this << " frame" << std::endl;


        frame->pts = av_rescale_q(presentationTimeSinceEpochMicroSec.count(), microSecBase, codecContext->time_base);

        // Make frame available to the encoder
        frameBuffer.push(frame);

		// Wait for new frame
		while (!face->getNewPixelsCondition().timed_wait(lock,
			boost::get_system_time() + boost::posix_time::milliseconds(100)))
		{
			if (destructing)
			{
				return;
			}
		}
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

boost::mutex triggerEventMutex;

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

		if (!pktPool.wait_and_pop(pkt))
		{
			// queue did close
			return;
		}

        //std::cout << this << " encode" << std::endl;
        
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
			abort();
		}

		x2yuv(xFrame, yuv420pFrame, codecContext);

		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;
		int got_output = 0;

		//mutex.lock();
		int ret = avcodec_encode_video2(codecContext, &pkt, yuv420pFrame, &got_output);
		//mutex.unlock();

		std::cout << "encoded frame" << std::endl;

//		fprintf(myfile, "packet size: %i \n", pkt.size);
	//	fflush(myfile);

		if (ret < 0)
		{
			fprintf(stderr, "Error encoding frame\n");
			return;
		}

		pkt.pts = xFrame->pts;

		framePool.push(xFrame);
		pktBuffer.push(pkt);

		//std::cout << this << ": encoded frame" << std::endl;

		//if (!this->destructing && isCurrentlyAwaitingData())
		{
			boost::mutex::scoped_lock lock(triggerEventMutex);
			envir().taskScheduler().triggerEvent(eventTriggerId, this);
			//std::cout << this << ": event triggered" << std::endl;
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
		//std::cout << this << ": deliver skipped" << std::endl;
		return; // we're not ready for the data yet
	}


	int64_t thisTime = av_gettime();

	//fprintf(myfile, "fMaxSize at beginning of function: %i \n", fMaxSize);
	//fflush(myfile);

	// set the duration of this frame since we have variable frame rate
	// %% Time has to be fixed
	//this->fDurationInMicroseconds = 1000000 / 70;// thisTime - lastFrameTime;

	
	//gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.

	//std::cout << this << ": pktBuffer size: " << pktBuffer.size() << std::endl;

	AVPacket pkt;
	if (!pktBuffer.wait_and_pop(pkt))
	{
		// queue did close
		return;
	}
    
    //std::cout << this << " send" << std::endl;

	// Set the presentation time of this frame
	AVRational secBase = { 1, 1 };
	AVRational microSecBase = { 1, 1000000 };
	fPresentationTime.tv_sec = av_rescale_q(pkt.pts, codecContext->time_base, secBase);
	fPresentationTime.tv_usec = av_rescale_q(pkt.pts, codecContext->time_base, microSecBase) -
		fPresentationTime.tv_sec * 1000000;

	// Live555 does not like start codes.
	// So, we remove the start code which is there in front of every nal unit.  
	// the start code might be 0x00000001 or 0x000001. so detect it and remove it.
	int truncateBytes = 0;
	if (pkt.size >= 4 &&
		pkt.data[0] == 0 &&
		pkt.data[1] == 0 &&
		pkt.data[2] == 0 &&
		pkt.data[3] == 1)
	{
		truncateBytes = 4;
	}
	else if (pkt.size >= 3 &&
		pkt.data[0] == 0 &&
		pkt.data[1] == 0 &&
		pkt.data[2] == 1)
	{
		truncateBytes = 3;
	}

	u_int8_t* newFrameDataStart = (u_int8_t*)(pkt.data + truncateBytes);
	unsigned newFrameSize = pkt.size - truncateBytes;

	//u_int8_t nal_unit_type = newFrameDataStart[0] & 0x1F;

	//if (nal_unit_type == 8) // PPS
	//{
	//	envir() << "PPS seen\n";
	//}
	//else if (nal_unit_type == 7) // SPS
	//{
	//	envir() << "SPS seen; siz\n";
	//}
	//else
	//{
	//	//envir() << nal_unit_type << " seen; size: " << frameSize << "\n";
	//}
	

	


	// Deliver the data here:
	if (newFrameSize > fMaxSize)
	{
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = newFrameSize - fMaxSize;
		//fprintf(myfile, "frameSize %i larger than maxSize %i\n", pkt.size, fMaxSize);
		//fflush(myfile);
	}
	else
	{
		fFrameSize = newFrameSize;
		/*rest =*/ fNumTruncatedBytes = 0;
		//
	}

	memmove(fTo, newFrameDataStart, fFrameSize);


	av_free_packet(&pkt);
	pktPool.push(pkt);

	if (fNumTruncatedBytes > 0)
	{
		std::cout << this << ": truncated " << fNumTruncatedBytes << " bytes" << std::endl;
	}

	// Tell live555 that a new frame is available
	FramedSource::afterGetting(this);

	std::cout << "sent frame" << std::endl;

	lastFrameTime = thisTime;

	//std::cout << this << ": delivered" << std::endl;
}
