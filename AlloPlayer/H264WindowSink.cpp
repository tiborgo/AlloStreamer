

#include <iostream>
#include "H264WindowSink.h"


H264WindowSink* H264WindowSink::createNew(UsageEnvironment& env,
	unsigned int bufferSize,
	MediaSubsession& subSession)
{
	return new H264WindowSink(env, bufferSize, subSession);
}

H264WindowSink::H264WindowSink(UsageEnvironment& env,
	unsigned int bufferSize,
	MediaSubsession& subSession)
	: MediaSink(env), bufferSize(bufferSize), buffer(new unsigned char[bufferSize]),
	subSession(subSession)
{

	frame = av_frame_alloc();
	if (!frame)
	{
		fprintf(stderr, "Could not allocate video frame\n");
		return;
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

	///* put sample parameters */
	//codecContext->bit_rate = bit_rate;
	///* resolution must be a multiple of two */
	//codecContext->width = face->width;
	//codecContext->height = face->height;
	///* frames per second */
	//codecContext->time_base = { 1, FPS };
	//codecContext->gop_size = 20; /* emit one intra frame every ten frames */
	//codecContext->max_b_frames = 0;
	//codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	//av_opt_set(codecContext->priv_data, "preset", preset_val, 0);
	//av_opt_set(codecContext->priv_data, "tune", tune_val, 0);

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		fprintf(stderr, "could not open codec\n");
		return;
	}
}

void H264WindowSink::afterGettingFrame(unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime)
{
	FramedSource* s = source();

	u_int8_t nal_unit_type;
	if (frameSize >= 1)
	{
		nal_unit_type = buffer[0] & 0x1F;

		if (nal_unit_type == 8) // PPS
		{
			envir() << "PPS seen; size: " << frameSize << "\n";
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
	int len, got_frame;
	AVPacket pkt;
	av_init_packet(&pkt);

	char* data = new char[frameSize + 4];
	pkt.size = frameSize;
	
	memcpy(data, start_code, 4);
	memcpy(data + 4, buffer, frameSize);
	pkt.data = (uint8_t*)data;
	len = avcodec_decode_video2(codecContext, frame, &got_frame, &pkt);

	if (len < 0)
	{
		//std::cout << this << ": error decoding frame" << std::endl;
	}
	else if (len == 0)
	{
		std::cout << this << ": no frame could be decoded" << std::endl;
	}
	else
	{
		std::cout << this << ": decoded frame (" << frame->width << ", " << frame->height << ")" << std::endl;
	}

	//std::cout << this << "frame" << std::endl;

	// Then try getting the next frame:
	continuePlaying();
}

Boolean H264WindowSink::continuePlaying()
{
	fSource->getNextFrame(buffer, bufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);

	return True;
}

void H264WindowSink::afterGettingFrame(void*clientData,
	unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime,
	unsigned durationInMicroseconds)
{
	H264WindowSink* sink = (H264WindowSink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}