

#include <iostream>
#include <map>
#include "H264RawPixelsSink.h"
#include <boost/thread.hpp>
#include <GroupsockHelper.hh>

namespace bc = boost::chrono;

H264RawPixelsSink* H264RawPixelsSink::createNew(UsageEnvironment& env,
	unsigned int bufferSize)
{
	return new H264RawPixelsSink(env, bufferSize);
}

H264RawPixelsSink::H264RawPixelsSink(UsageEnvironment& env,
	unsigned int bufferSize)
	: MediaSink(env), bufferSize(bufferSize), buffer(new unsigned char[bufferSize]),
	img_convert_ctx(NULL)
{
	for (int i = 0; i < 1; i++)
	{
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = AV_PIX_FMT_RGBA;

		framePool.push(frame);

		AVPacket* pkt = new AVPacket;
		pktPool.push(pkt);
	}


	// Initialize codec and decoder
	AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		fprintf(stderr, "Codec not found\n");
		return;
	}

	codecContext = avcodec_alloc_context3(codec);

	if (!codecContext)
	{
		fprintf(stderr, "could not allocate video codec context\n");
		return;
	}

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		fprintf(stderr, "could not open codec\n");
		return;
	}

	decodeFrameThread = boost::thread(boost::bind(&H264RawPixelsSink::decodeFrameLoop, this));
}

void H264RawPixelsSink::afterGettingFrame(unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime)
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	long relativePresentationTimeMicroSec = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec -
		(currentTime.tv_sec * 1000000 + currentTime.tv_usec);

	long acceptedDelayMicroSec = 100000; // 10000 milliseconds

	if (true /*relativePresentationTimeMicroSec + acceptedDelayMicroSec >= 0*/)
	{
		


		u_int8_t nal_unit_type;
		if (frameSize >= 1)
		{
			nal_unit_type = buffer[0] & 0x1F;



			if (nal_unit_type == 8) // PPS
			{
				//envir() << "PPS seen; size: " << frameSize << "\n";
			}
			else if (nal_unit_type == 7) // SPS
			{
				//envir() << "SPS seen; size: " << frameSize << "\n";
			}
			else
			{
				//envir() << nal_unit_type << " seen; size: " << frameSize << "\n";
			}
		}

		

		//envir() << "sprop - parameter - sets: " << subSession.fmtp_spropparametersets() << "\n";
		unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
		unsigned char const end_code[2] = { 0x00, 0x00 };
		
		AVPacket* pkt;
		if (!pktPool.wait_and_pop(pkt))
		{
			// queue did close
			return;
		}

		av_init_packet(pkt);

		AVRational microSecBase = { 1, 1000000 };

		pkt->size = frameSize + sizeof(start_code);
		pkt->data = (uint8_t*)new char[frameSize + sizeof(start_code)];
		pkt->pts = av_rescale_q(presentationTime.tv_sec * 1000000 + presentationTime.tv_usec,
			microSecBase,
			codecContext->time_base);

		memcpy(pkt->data, start_code, sizeof(start_code));
		memcpy(pkt->data + sizeof(start_code), buffer, frameSize);

		pktBuffer.push(pkt);

		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);

		long relativePresentationTimeMicroSec = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec -
			(currentTime.tv_sec * 1000000 + currentTime.tv_usec);

		//std::cout << this << " " << -relativePresentationTimeMicroSec / 1000.0 << " milliseconds to late" << std::endl;
	}
	else
	{
		//std::cout << this << " skipped frame" << std::endl;
	}

	// Then try getting the next frame:
	continuePlaying();

	

	
}

Boolean H264RawPixelsSink::continuePlaying()
{
	fSource->getNextFrame(buffer, bufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);

	return True;
}

void H264RawPixelsSink::afterGettingFrame(void*clientData,
	unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime,
	unsigned durationInMicroseconds)
{
	H264RawPixelsSink* sink = (H264RawPixelsSink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

void H264RawPixelsSink::decodeFrameLoop()
{


	while (true)
	{
		// Pop frame ptr from buffer
		AVFrame* frame;
		AVFrame* yuvFrame;
		AVPacket* pkt;


		if (!pktBuffer.wait_and_pop(pkt))
		{
			// queue did close
			return;
		}

		if (!framePool.wait_and_pop(frame))
		{
			// queue did close
			return;
		}

		yuvFrame = av_frame_alloc();
		if (!yuvFrame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		int got_frame;
		int len = avcodec_decode_video2(codecContext, yuvFrame, &got_frame, pkt);

		if (len < 0)
		{
			//std::cout << this << ": error decoding frame" << std::endl;
		}
		else if (len == 0)
		{
			std::cout << this << ": no frame could be decoded" << std::endl;
		}
		else if (got_frame == 1)
		{
			if (!frame->data[0])
			{
				frame->width = yuvFrame->width;
				frame->height = yuvFrame->height;

				/* the image can be allocated by any means and av_image_alloc() is
				* just the most convenient way if av_malloc() is to be used */
				if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
					(AVPixelFormat)frame->format, 32) < 0)
				{
					fprintf(stderr, "Could not allocate raw picture buffer\n");
					exit(1);
				}
			}

			//x2y(yuvFrame, frame, codecContext);

			if (!img_convert_ctx)
			{
				img_convert_ctx = sws_getContext(
					yuvFrame->width, yuvFrame->height, (AVPixelFormat)yuvFrame->format,
					frame->width, frame->height, (AVPixelFormat)frame->format,
					SWS_BICUBIC, NULL, NULL, NULL);
			}

			int x = sws_scale(img_convert_ctx, yuvFrame->data,
				yuvFrame->linesize, 0, yuvFrame->height,
				frame->data, frame->linesize);

			// Flip image vertically
			/*for (int i = 0; i < 4; i++)
			{
				frame->data[i] += frame->linesize[i] * (frame->height - 1);
				frame->linesize[i] = -frame->linesize[i];
			}*/

			//std::cout << this << ": afterGettingFrame: " << frame << "\n";

			//printf("%ld.%06ld\n", presentationTime.tv_sec, presentationTime.tv_usec);

			//av_freep(&yuvFrame->data[0]);
			av_frame_free(&yuvFrame);
		}

		bc::microseconds nowSinceEpoch =
			bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());

		AVRational microSecBase = { 1, 1000000 };
		bc::microseconds presentationTimeSinceEpoch =
			bc::microseconds(av_rescale_q(pkt->pts, codecContext->time_base, microSecBase));


		bc::microseconds relativePresentationTime = presentationTimeSinceEpoch - nowSinceEpoch;

		sumRelativePresentationTimeMicroSec += relativePresentationTime.count();
		if (maxRelativePresentationTimeMicroSec > relativePresentationTime.count())
		{
			maxRelativePresentationTimeMicroSec = relativePresentationTime.count();
		}

		const long frequency = 100;
		if (counter % frequency == 0)
		{
			std::cout << this << " delay: avg " << -sumRelativePresentationTimeMicroSec / 1000.0 / frequency << " ms; max " << -maxRelativePresentationTimeMicroSec / 1000.0 << " ms" << std::endl;
			sumRelativePresentationTimeMicroSec = 0;
			maxRelativePresentationTimeMicroSec = 0;
		}

		counter++;

		frameBuffer.push(frame) ;
		pktPool.push(pkt);
	}
}

Frame* H264RawPixelsSink::getNextFrame()
{
	AVFrame* avFrame;

	frameBuffer.wait_and_pop(avFrame);

	//boost::interprocess::managed_heap_memory heapMemory(avFrame->width * avFrame->height * 4 + 1024);
	//Allocator<HeapSegmentManager> heapAllocator(heapMemory.get_segment_manager());
	//Frame* frame = new Frame(avFrame->width, avFrame->height, (AVPixelFormat)avFrame->format, heapAllocator);
	
	unsigned char* pixels = new unsigned char[avFrame->width * avFrame->height * 4];

	int ret = avpicture_layout((AVPicture*)avFrame, (AVPixelFormat)avFrame->format,
		avFrame->width, avFrame->height, pixels/*(unsigned char*)frame->pixels.get()*/, avFrame->width * avFrame->height * 4);


	framePool.push(avFrame);

	return (Frame*)pixels;
}