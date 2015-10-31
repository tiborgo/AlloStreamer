#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <stdint.h>
#include <time.h>
#include <boost/thread.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "config.h"
#include "RawPixelSource.hpp"

namespace bc = boost::chrono;

boost::mutex RawPixelSource::triggerEventMutex;
std::vector<RawPixelSource*> RawPixelSource::sourcesReadyForDelivery;

int RawPixelSource::x2yuv(AVFrame *xFrame, AVFrame *yuvFrame, AVCodecContext *c)
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
	for (int i = 0; i < 4; i++)
	{
		if (xFrame->linesize[i] > 0)
		{
			xFrame->data[i] += xFrame->linesize[i] * (c->height - 1);
			xFrame->linesize[i] = -xFrame->linesize[i];
		}
	}
	return sws_scale(img_convert_ctx, xFrame->data,
		xFrame->linesize, 0, c->height,
		yuvFrame->data, yuvFrame->linesize);
}

RawPixelSource* RawPixelSource::createNew(UsageEnvironment& env,
                                          Frame* content,
                                          int avgBitRate,
										  bool robustSyncing)
{
	return new RawPixelSource(env, content, avgBitRate, robustSyncing);
}

unsigned RawPixelSource::referenceCount = 0;

struct timeval prevtime;

RawPixelSource::RawPixelSource(UsageEnvironment& env,
                               Frame* content,
							   int avgBitRate,
							   bool robustSyncing)
	:
	FramedSource(env), img_convert_ctx(NULL), content(content), /*encodeBarrier(2),*/ destructing(false), lastPTS(0), robustSyncing(robustSyncing)
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
		frame->format = content->getFormat();
		frame->width  = content->getWidth();
		frame->height = content->getHeight();

		/* the image can be allocated by any means and av_image_alloc() is
		* just the most convenient way if av_malloc() is to be used */
		if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
			content->getFormat(), 32) < 0)
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
	codecContext->width = content->getWidth();
	codecContext->height = content->getHeight();
	/* frames per second */
	codecContext->time_base = av_make_q(1, FPS);
	codecContext->gop_size = 20; /* emit one intra frame every ten frames */
	codecContext->max_b_frames = 0;
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	//codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

	av_opt_set(codecContext->priv_data, "preset", PRESET_VAL, 0);
	av_opt_set(codecContext->priv_data, "tune", TUNE_VAL, 0);
	av_opt_set(codecContext->priv_data, "slice-max-size", "2000", 0);

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

	frameContentThread = boost::thread(boost::bind(&RawPixelSource::frameContentLoop, this));

	encodeFrameThread  = boost::thread(boost::bind(&RawPixelSource::encodeFrameLoop,  this));

	//eventThread        = boost::thread(boost::bind(&RawPixelSource::eventLoop, this));

	lastFrameTime = av_gettime();
}

RawPixelSource::~RawPixelSource()
{
	// Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
	//std::cout << this << ": deconstructing..." << std::endl;

	this->destructing = true;
	frameBuffer.close();
	pktBuffer.close();
	framePool.close();
	pktPool.close();

	frameContentThread.join();
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

void RawPixelSource::setOnSentNALU(const OnSentNALU& callback)
{
	onSentNALU = callback;
}

void RawPixelSource::setOnEncodedFrame(const OnEncodedFrame& callback)
{
	onEncodedFrame = callback;
}

void RawPixelSource::frameContentLoop()
{

	while (!destructing)
	{

		AVFrame* frame;
		if (!framePool.wait_and_pop(frame))
		{
			return;
		}

		// End this thread when CubemapExtractionPlugin closes
		while (!content->getBarrier().timedWait(boost::chrono::milliseconds(100)))
		{
			if (destructing)
			{
				return;
			}
		}

		//content->getBarrier().wait();

		AVRational microSecBase = { 1, 1000000 };
		bc::microseconds presentationTimeSinceEpochMicroSec;

		int_least64_t x;
		{
			boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(content->getMutex());

			// Fill frame
			avpicture_fill((AVPicture*)frame,
				(uint8_t*)content->getPixels(),
				content->getFormat(),
				content->getWidth(),
				content->getHeight());

			// Set the actual presentation time
			// It is in the past probably but we will try our best
			
			presentationTimeSinceEpochMicroSec =
				bc::duration_cast<bc::microseconds>(content->getPresentationTime().time_since_epoch());

			x = content->getPresentationTime().time_since_epoch().count();
		}
        
		

//        const time_t time = bc::system_clock::to_time_t(face->getPresentationTime());
//
//        // Maybe the put_time will be implemented later?
//        struct tm* tm = localtime(&time);
//        char timeStr[128];
//        strftime (timeStr, sizeof(timeStr), "%c",tm);
        
        //std::cout << this << " presentation time: " << timeStr << std::endl;
        //std::cout << this << " frame" << std::endl;


		frame->pts = presentationTimeSinceEpochMicroSec.count();// av_rescale_q(presentationTimeSinceEpochMicroSec.count(), microSecBase, codecContext->time_base);

		if (x == lastPTS)
		{
			std::cout << "match!?" << std::endl;
		}

		lastPTS = frame->pts;

		//std::cout << presentationTimeSinceEpochMicroSec.count() << " " << x << " " << frame->pts << std::endl;

        // Make frame available to the encoder
        frameBuffer.push(frame);
	}
}

void RawPixelSource::doGetNextFrame()
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

void RawPixelSource::deliverFrame0(void* clientData)
{
	boost::mutex::scoped_lock lock(triggerEventMutex);
	for (RawPixelSource* source : sourcesReadyForDelivery)
	{
		source->deliverFrame();
	}
	sourcesReadyForDelivery.clear();
	//std::cout << "deliver frame: " << ((CubemapFaceSource*)clientData)->face->index << std::endl;
}

void RawPixelSource::encodeFrameLoop()
{
	while (!this->destructing)
	{
		AVPacket pkt;
		int64_t pts;

		{
			// Pop frame ptr from buffer
			AVFrame* xFrame;
			AVFrame* yuv420pFrame;

			if (!frameBuffer.wait_and_pop(xFrame))
			{
				// queue did close
				return;
			}

			pts = xFrame->pts;

			//std::cout << this << " encode" << std::endl;

			if (xFrame->format != AV_PIX_FMT_YUV420P)
			{
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
			}
			else
			{
				yuv420pFrame = xFrame;
			}

			av_init_packet(&pkt);
			pkt.data = NULL; // packet data will be allocated by the encoder
			pkt.size = 0;
			int got_output = 0;

			//mutex.lock();
			int ret = avcodec_encode_video2(codecContext, &pkt, yuv420pFrame, &got_output);
			if (ret < 0)
			{
				fprintf(stderr, "Error encoding frame\n");
				abort();
			}

			if (onEncodedFrame) onEncodedFrame(this);

			framePool.push(xFrame);

			if (xFrame->format != AV_PIX_FMT_YUV420P)
			{
				av_freep(&yuv420pFrame->data[0]);
				av_frame_free(&yuv420pFrame);
			}
		}
	
		{
			// pair.first: pos if first byte of NALU; pair.second: pos of last byte of NALU
			std::queue<std::pair<size_t, size_t> > naluPoses;

			// Parse package for all NALUs
			size_t naluStartPos = 0;
			for (size_t i = 0; i < pkt.size - 3; i++)
			{
				if (pkt.data[i] == 0 &&
					pkt.data[i + 1] == 0)
				{
					if (pkt.data[i + 2] == 0 &&
						pkt.data[i + 3] == 1)
					{
						if (i != 0)
						{
							naluPoses.push(std::make_pair(naluStartPos, i - 1));
						}

						naluStartPos = i + 4;
						i += 3;
					}
					else if (pkt.data[i + 2] == 1)
					{
						if (i != 0)
						{
							naluPoses.push(std::make_pair(naluStartPos, i - 1));
						}

						naluStartPos = i + 3;
						i += 2;
					}
				}
			}
			naluPoses.push(std::make_pair(naluStartPos, pkt.size - 1));


			size_t naluCount = naluPoses.size();


			AVPacket dummy;
			if (!pktPool.wait_and_pop(dummy))
			{
				// queue did close
				return;
			}

			for (size_t i = 0; i < naluCount; i++)
			{
				std::pair<size_t, size_t> naluPos = naluPoses.front();
				naluPoses.pop();

				AVPacket naluPkt;
				int naluPktSize = naluPos.second - naluPos.first + 1;
				if (robustSyncing)
				{
					naluPktSize += sizeof(int64_t);
				}
				av_new_packet(&naluPkt, naluPktSize);
				memcpy(naluPkt.data, pkt.data + naluPos.first, naluPos.second - naluPos.first + 1);
				if (robustSyncing)
				{
					*((int64_t*)(naluPkt.data + naluPos.second - naluPos.first + 1)) = pts;
				}
				//std::cout << *((int64_t*)(naluPkt.data + naluPos.second - naluPos.first + 1)) << std::endl;
				naluPkt.pts = pts;

				pktBuffer.push(naluPkt);

				{
					boost::mutex::scoped_lock lock(triggerEventMutex);
					sourcesReadyForDelivery.push_back(this);
					envir().taskScheduler().triggerEvent(eventTriggerId, nullptr);
				}
			}

			av_free_packet(&pkt);
		}
	}
}

void RawPixelSource::deliverFrame()
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

	//std::cout << "available size: " << fMaxSize << std::endl;

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

	if (pktBuffer.size() == 0)
	{
		AVPacket dummy;
		pktPool.push(dummy);
	}
    
    //std::cout << this << " send" << std::endl;

	// Set the presentation time of this frame
	AVRational secBase = { 1, 1 };
	AVRational microSecBase = { 1, 1000000 };
	fPresentationTime.tv_sec = pkt.pts / 1000000; //av_rescale_q(pkt.pts, codecContext->time_base, secBase);
	fPresentationTime.tv_usec = pkt.pts % 1000000; // av_rescale_q(pkt.pts, codecContext->time_base, microSecBase) -

	//std::cout << fPresentationTime.tv_sec << " " << fPresentationTime.tv_usec << std::endl;

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

	u_int8_t* newFrameDataStart = (u_int8_t*)(pkt.data /*+ truncateBytes*/);
	unsigned newFrameSize = pkt.size/* - truncateBytes*/;

	if ((int)(pkt.data[0] & 0x1F) == 5)
	{
		//std::cout << newFrameSize << std::endl;
	}
	

	u_int8_t nal_unit_type = newFrameDataStart[0] & 0x1F;

	//std::cout << "sent NALU type " << (int)nal_unit_type << " (" << newFrameSize << ")" << std::endl;

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
	//pktPool.push(pkt);

	if (fNumTruncatedBytes > 0)
	{
		std::cout << this << ": truncated " << fNumTruncatedBytes << " bytes" << std::endl;
	}

	//std::cout << fFrameSize << std::endl;

	// Tell live555 that a new frame is available
	FramedSource::afterGetting(this);

	//std::cout << pkt.pts << std::endl;

	if (onSentNALU) onSentNALU(this, nal_unit_type, fFrameSize);

	//std::cout << "sent frame" << std::endl;

	lastFrameTime = thisTime;

	//std::cout << this << ": delivered" << std::endl;

	//boost::this_thread::sleep_for(boost::chrono::microseconds((size_t)(1 * fFrameSize)));
}
