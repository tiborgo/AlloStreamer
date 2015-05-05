

#include <iostream>
#include <map>
#include "H264RawPixelsSink.h"
#include <boost/thread.hpp>
#include <GroupsockHelper.hh>

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
		frame->format = AV_PIX_FMT_BGRA;

		framePool.push(frame);
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

	if (relativePresentationTimeMicroSec + acceptedDelayMicroSec >= 0)
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
		int len, got_frame;
		AVPacket pkt;
		av_init_packet(&pkt);

		char* data = new char[frameSize + sizeof(start_code)/* + sizeof(end_code)*/];
		pkt.size = frameSize + sizeof(start_code);// +5;// + sizeof(end_code);

		memcpy(data, start_code, sizeof(start_code));
		memcpy(data + sizeof(start_code), buffer, frameSize);
		//memcpy(data + sizeof(start_code)+frameSize, end_code, sizeof(end_code));
		pkt.data = (uint8_t*)data;

		AVFrame* yuvFrame = av_frame_alloc();
		if (!yuvFrame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		len = avcodec_decode_video2(codecContext, yuvFrame, &got_frame, &pkt);
		//got_frame = 1;

		//envir() << "(" << nal_unit_type << ", " << frameSize << ", " << len << ", " << got_frame << ")\n";

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
			//std::cout << this << ": decoded frame (" << yuvFrame->width << ", " << yuvFrame->height << ")" << std::endl;


			AVFrame* frame;
			framePool.wait_and_pop(frame);

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
			for (int i = 0; i < 4; i++)
			{
				frame->data[i] += frame->linesize[i] * (frame->height - 1);
				frame->linesize[i] = -frame->linesize[i];
			}

			//std::cout << this << ": afterGettingFrame: " << frame << "\n";

			//printf("%ld.%06ld\n", presentationTime.tv_sec, presentationTime.tv_usec);

			//av_freep(&yuvFrame->data[0]);
			av_frame_free(&yuvFrame);

			frameBuffer.push(frame);
		}

		//std::cout << this << "frame" << std::endl;

		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);

		long relativePresentationTimeMicroSec = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec -
			(currentTime.tv_sec * 1000000 + currentTime.tv_usec);

		std::cout << this << " " << -relativePresentationTimeMicroSec / 1000.0 << " milliseconds to late" << std::endl;
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