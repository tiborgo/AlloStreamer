

#include <iostream>
#include <map>
#include <boost/thread.hpp>
#include <GroupsockHelper.hh>

#include "H264NALUSink.hpp"

namespace bc = boost::chrono;

const size_t MAX_NALUS_PER_PKT = 30;
unsigned char const START_CODE[4] = { 0x00, 0x00, 0x00, 0x01 };
const size_t MAX_NALU_SIZE = 1000000;
const size_t MAX_PKT_SIZE  = (sizeof(START_CODE) + MAX_NALU_SIZE) * MAX_NALUS_PER_PKT;

H264NALUSink* H264NALUSink::createNew(UsageEnvironment& env,
                                      unsigned long     bufferSize,
                                      AVPixelFormat     format,
                                      MediaSubsession*  subsession,
                                      bool              robustSyncing)
{
    av_log_set_level(AV_LOG_FATAL);
    avcodec_register_all();
    avformat_network_init();
	return new H264NALUSink(env, bufferSize, format, subsession, robustSyncing);
}

void H264NALUSink::setOnReceivedNALU(const OnReceivedNALU& callback)
{
    onReceivedNALU = callback;
}

void H264NALUSink::setOnReceivedFrame(const OnReceivedFrame& callback)
{
    onReceivedFrame = callback;
}

void H264NALUSink::setOnDecodedFrame(const OnDecodedFrame& callback)
{
    onDecodedFrame = callback;
}

void H264NALUSink::setOnColorConvertedFrame(const OnColorConvertedFrame& callback)
{
    onColorConvertedFrame = callback;
}

H264NALUSink::H264NALUSink(UsageEnvironment& env,
                           unsigned int      bufferSize,
                           AVPixelFormat     format,
                           MediaSubsession*  subsession,
                           bool              robustSyncing)
    :
    MediaSink(env), bufferSize(bufferSize), buffer(new unsigned char[bufferSize]),
    imageConvertCtx(NULL), receivedFirstPriorityPackages(false), format(format),
    counter(0), sumRelativePresentationTimeMicroSec(0), maxRelativePresentationTimeMicroSec(0), subsession(subsession), lastTotal(0),
    pts(-1), lastPTS(-1), robustSyncing(robustSyncing)
{
    for (int i = 0; i < MAX_NALUS_PER_PKT + 1; i++)
    {
        naluPool.push(new NALU({new unsigned char[MAX_NALU_SIZE], 0, -1}));
    }
    
    for (int i = 0; i < 5; i++)
    {
        AVPacket* pkt = new AVPacket;
        av_new_packet(pkt, MAX_PKT_SIZE);
        pkt->size = 0;
        pktPool.push(pkt);
    }
    pktPool.waitAndPop(currentPkt);
    
	for (int i = 0; i < 60; i++)
	{
        
        
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			abort();
		}
		framePool.push(frame);
        
        AVFrame* resizedFrame = av_frame_alloc();
        if (!resizedFrame)
        {
            fprintf(stderr, "Could not allocate video frame\n");
			abort();
        }
        resizedFrame->format = format;
        convertedFramePool.push(resizedFrame);
	}


	// Initialize codec and decoder
	AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		fprintf(stderr, "Codec not found\n");
		abort();
	}

	codecContext = avcodec_alloc_context3(codec);

	if (!codecContext)
	{
		fprintf(stderr, "could not allocate video codec context\n");
		abort();
	}

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		fprintf(stderr, "could not open codec\n");
		abort();
	}

    //packageNALUsThread = boost::thread(boost::bind(&H264NALUSink::packageNALUsLoop, this));
	decodeFrameThread  = boost::thread(boost::bind(&H264NALUSink::decodeFrameLoop,  this));
    convertFrameThread = boost::thread(boost::bind(&H264NALUSink::convertFrameLoop, this));
}

void H264NALUSink::packageData(AVPacket* pkt, unsigned int frameSize, timeval presentationTime)
{
    unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
    //unsigned char const start_code[0] = { };
    
    av_init_packet(pkt);
    
    AVRational microSecBase = { 1, 1000000 };
    
    pkt->size = frameSize + sizeof(start_code);
    pkt->data = (uint8_t*)new char[frameSize + sizeof(start_code)];
    pkt->pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
    
    memcpy(pkt->data, start_code, sizeof(start_code));
    memcpy(pkt->data + sizeof(start_code), buffer, frameSize);
}

void H264NALUSink::afterGettingFrame(unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime)
{
    //std::cout << subsession->fmtp_spropparametersets() << std::endl;
    
    //std::cout << "Received NALU" << std::endl;
    
    //std::cout << this << " " << presentationTime.tv_sec << " " << presentationTime.tv_usec << std::endl;
    
    u_int8_t nal_unit_type = buffer[0] & 0x1F;

	/*if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, frameSize);

	continuePlaying();
	return;*/
    
    //std::cout << naluPool.size() << std::endl;
    
    /*pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
    int64_t altPts;
    for (int i = 0; i < sizeof(int64_t); i++)
    {
        ((char*)&altPts)[i] = *(buffer + frameSize - (8-i));
    }
    altPts = *((int64_t*)(buffer + frameSize - sizeof(int64_t)));
    std::cout << altPts << " " << pts << std::endl;*/
    
    size_t packageSize;
    if (robustSyncing && frameSize >= sizeof(int64_t))
    {
        pts = *((int64_t*)(buffer + frameSize - sizeof(int64_t)));
        packageSize = frameSize - sizeof(int64_t);
    }
    else
    {
        pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
        packageSize = frameSize;
    }
    
    // Check if all NALUs for current frame have arrived
    if (lastPTS != -1 && lastPTS != pts)
    {
        if (onReceivedFrame) onReceivedFrame(this, currentPkt->data[4] & 0x1F, currentPkt->size);
        
        // make frame available to the decoder
        // if we currently have the capacities to encode another frame
        AVPacket* pkt;
        if (pktPool.tryPop(pkt))
        {
            pktBuffer.push(currentPkt);
            currentPkt = pkt;
        }
        
        // Reset current pkt so that we can fill it with new NALUs
        currentPkt->size = 0;
    }
    
    // Add NALU to current frame pkt
    if (sizeof(START_CODE) + packageSize > MAX_PKT_SIZE)
    {
        std::cout << "NALUs are too big for one pkt!" << std::endl;
    }
    else
    {
        memcpy(currentPkt->data + currentPkt->size, START_CODE, sizeof(START_CODE));
        memcpy(currentPkt->data + currentPkt->size + sizeof(START_CODE), buffer, packageSize);
        currentPkt->size += sizeof(START_CODE) + packageSize;
        currentPkt->pts = pts;
    }
    
    lastPTS = pts;
    
//    NALU* nalu;
//    if (naluPool.tryPop(nalu))
//    {
//        size_t size;
//        if (frameSize > MAX_NALU_SIZE)
//        {
//            std::cout << "NALU too big!" << std::endl;
//            size = MAX_NALU_SIZE;
//        }
//        else
//        {
//            size = frameSize;
//        }
//        memcpy(nalu->buffer, buffer, size);
//        nalu->size = size;
//        nalu->pts  = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
//        naluBuffer.push(nalu);
//        //if (onAddedNALU) onAddedNALU(this, nal_unit_type, frameSize);
//    }
//    else
//    {
//        if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, frameSize);
//    }
    

    //std::cout << "type: " << (int)nal_unit_type << " " << frameSize << std::endl;
    
//    static uint64_t lastPTS = -1;
//    static std::deque<std::string> packets;
//    static size_t frameNALUsSize = 0;
//    
//    uint64_t pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
//    
//    if (lastPTS != -1 && pts != lastPTS)
//    {
//        unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
//        size_t size = frameNALUsSize + packets.size() * sizeof(start_code);
//        
//        AVPacket* pkt;
//        
//        if (pktPool.tryPop(pkt))
//        {
//            
//            av_init_packet(pkt);
//            
//            pkt->size = size;
//            pkt->data = (uint8_t*)new char[pkt->size];
//            pkt->pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
//            
//            uint8_t* dataPtr = pkt->data;
//            while (packets.size() > 0)
//            {
//                std::string& packet = packets.front();
//                memcpy(dataPtr, start_code, sizeof(start_code));
//                memcpy(dataPtr + sizeof(start_code), packet.data(), packet.size());
//                dataPtr += packet.size() + sizeof(start_code);
//                packets.pop_front();
//            }
//            
//
//            pktBuffer.push(pkt);
//            if (onAddedNALU) onAddedNALU(this, nal_unit_type, size);
//        }
//        else
//        {
//            if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, size);
//        }
//        
//        frameNALUsSize = 0;
//        packets.clear();
//    }
//    
//    std::string packet((char*)buffer, frameSize);
//    packets.push_back(packet);
//    frameNALUsSize += frameSize;
//    lastPTS = pts;
    
    
    
    //std::cout << "type " << (int)nal_unit_type << " " << frameSize << std::endl;
    
    // Then try getting the next frame:
    continuePlaying();
    return;
    
//    if (pkt)
//    {
//        // We currently have the capacities to decode the received frame
//        
//        if (priorityPackages.size() > 0)
//        {
//            // We still have SPS, PPS and/or IDR-slices to process -> do it now
//            // Don't care about the received frame
//            pktBuffer.push(priorityPackages.front());
//            priorityPackages.pop();
//            receivedFirstPriorityPackages = true;
//            delete pkt; // avoid memory leak
//            if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, frameSize);
//        }
//        else if (nal_unit_type == 1 && receivedFirstPriorityPackages)
//        {
//            // We received a B/P frame
//            // and have the capacities to decode it -> do it
//            packageData(pkt, frameSize, presentationTime);
//            pktBuffer.push(pkt);
//			if (onAddedNALU) onAddedNALU(this, nal_unit_type, frameSize);
//        }
//        else if (nal_unit_type == 7 /* SPS */ ||
//                 nal_unit_type == 8 /* PPS */ ||
//                 nal_unit_type == 5 /* IDR-slice */)
//        {
//            packageData(pkt, frameSize, presentationTime);
//            pktBuffer.push(pkt);
//            receivedFirstPriorityPackages = true;
//			if (onAddedNALU) onAddedNALU(this, nal_unit_type, frameSize);
//        }
//        else
//        {
//            pktPool.push(pkt);
//			if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, frameSize);
//        }
//    }
//    else
//    {
//        // We currently don't have the capacities to decode the received frame
//        
//        if (nal_unit_type == 7 /* SPS */ ||
//            nal_unit_type == 8 /* PPS */ ||
//            nal_unit_type == 5 /* IDR-slice */)
//        {
//            // We received a priority NALU but don't have the capacities
//            // to encode it right now -> safe it for later
//            AVPacket* priorityNALU = new AVPacket;
//            packageData(priorityNALU, frameSize, presentationTime);
//            priorityPackages.push(priorityNALU);
//        }
//        else if (nal_unit_type == 1)
//        {
//            // We received a B/P frame but don't have the capacities
//            // to encode it right now.
//            // We can safely skip it.
//            
//            if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, frameSize);
//        }
//    }
//
//	// Then try getting the next frame:
//	continuePlaying();
}

Boolean H264NALUSink::continuePlaying()
{
	fSource->getNextFrame(buffer, bufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);

	return True;
}

void H264NALUSink::afterGettingFrame(void*clientData,
                                          unsigned frameSize,
                                          unsigned numTruncatedBytes,
                                          timeval presentationTime,
                                          unsigned durationInMicroseconds)
{
	H264NALUSink* sink = (H264NALUSink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

void H264NALUSink::packageNALUsLoop()
{
    int64_t lastPTS = -1;
    int64_t pts     = -1;
    std::queue<NALU*> nalus;
    
    /*while (true)
    {
        NALU* nalu;
        if (!naluBuffer.waitAndPop(nalu))
        {
            // queue did close
            return;
        }
        naluPool.push(nalu);
    }*/
    
    while (true)
    {
        AVPacket* pkt;
        if (!pktPool.waitAndPop(pkt))
        {
            // queue did close
            return;
        }
        
        do
        {
            if (nalus.size() == MAX_NALUS_PER_PKT + 1)
            {
                std::cerr << "Too many NALUs per pkt!" << std::endl;
                abort();
            }
            
            lastPTS = pts;
            
            NALU* nalu;
            if (!naluBuffer.waitAndPop(nalu))
            {
                // queue did close
                return;
            }
            nalus.push(nalu);
            pts = nalu->pts;
        }
        while (lastPTS == -1 || pts == lastPTS);
        
        pkt->size = 0;
        
        while (nalus.size() > 1)
        {
            NALU* nalu = nalus.front();
            nalus.pop();
            
            if (sizeof(START_CODE) + nalu->size + pkt->size > MAX_PKT_SIZE)
            {
                std::cout << "NALUs are too big for one pkt!" << std::endl;
            }
            else
            {
                memcpy(pkt->data + pkt->size, START_CODE, sizeof(START_CODE));
                memcpy(pkt->data + pkt->size + sizeof(START_CODE), nalu->buffer, nalu->size);
                pkt->size += sizeof(START_CODE) + nalu->size;
                pkt->pts = lastPTS;
            }
            
            naluPool.push(nalu);
        }
        
        pktBuffer.push(pkt);
        //pktPool.push(pkt);
            
//            if (lastPTS != -1 && nalu->pts != lastPTS)
//                    {
//                        unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
//                        size_t size = frameNALUsSize + packets.size() * sizeof(start_code);
//                
//                        AVPacket* pkt;
//                
//                        if (pktPool.tryPop(pkt))
//                        {
//                
//                            av_init_packet(pkt);
//                
//                            pkt->size = size;
//                            pkt->data = (uint8_t*)new char[pkt->size];
//                            pkt->pts = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec;
//                
//                            uint8_t* dataPtr = pkt->data;
//                            while (packets.size() > 0)
//                            {
//                                std::string& packet = packets.front();
//                                memcpy(dataPtr, start_code, sizeof(start_code));
//                                memcpy(dataPtr + sizeof(start_code), packet.data(), packet.size());
//                                dataPtr += packet.size() + sizeof(start_code);
//                                packets.pop_front();
//                            }
//                
//                
//                            pktBuffer.push(pkt);
//                            if (onAddedNALU) onAddedNALU(this, nal_unit_type, size);
//                        }
//                        else
//                        {
//                            if (onDroppedNALU) onDroppedNALU(this, nal_unit_type, size);
//                        }
//                        
//                        frameNALUsSize = 0;
//                        packets.clear();
//                    }
//            
//        
//        }
//        while ();
    }
}

void H264NALUSink::decodeFrameLoop()
{
	while (true)
	{
		// Pop frame ptr from buffer
		AVFrame* frame;
		AVPacket* pkt;

		if (!pktBuffer.waitAndPop(pkt))
		{
			// queue did close
			return;
		}
        //std::cout << pktPool.size() << std::endl;

		if (!framePool.waitAndPop(frame))
		{
			// queue did close
			return;
		}
        //std::cout << framePool.size() << std::endl;

		int got_frame;
		int len = avcodec_decode_video2(codecContext, frame, &got_frame, pkt);
        
        //std::cout << "len " << len - pkt->size << std::endl;
        //std::cout << "type: " << int(pkt->data[4] & 0x1F) << std::endl;
        //std::cout << "time " << pkt->pts << std::endl;
        
		pktPool.push(pkt);

        if (got_frame == 1)
        {
            if (onDecodedFrame) onDecodedFrame(this,
                                               frame->key_frame,
                                               avpicture_get_size((AVPixelFormat)frame->format,
                                                                  frame->width,
                                                                  frame->height));
            //std::cout << "got frame" << std::endl;
            
            // We have decoded a frame :) ->
            // Make the frame available to the application
            frame->pts = pkt->pts;
            
            static uint64_t last = 0;
            
            uint64_t t = frame->pts;
            
            //std::cout << t - last << std::endl;
            last = t;
            
            //std::cout << this << " " << frame->pts << std::endl;
            
            frameBuffer.push(frame);
            //framePool.push(frame);
            
            //std::cout << "frame" << std::endl;
		}
        else
        {
            //std::cout << "didn't get frame" << std::endl;
            
            // No frame could be decoded :( ->
            // Put frame back to the pool so that the next packet will be read
            framePool.push(frame);
            
            if (len < 0)
            {
                // error decoding frame
            }
            else if (len == 0)
            {
                // package contained no frame
            }
            
            //std::cout << "no frame" << std::endl;
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
			//std::cout << this << " delay: avg " << -sumRelativePresentationTimeMicroSec / 1000.0 / frequency << " ms; max " << -maxRelativePresentationTimeMicroSec / 1000.0 << " ms" << std::endl;
			sumRelativePresentationTimeMicroSec = 0;
			maxRelativePresentationTimeMicroSec = 0;
            
            //std::cout << stats.summary(bc::milliseconds(1001)) << std::endl;
		}

		counter++;
		
	}
}

void H264NALUSink::convertFrameLoop()
{
    while(true)
    {
        AVFrame* frame;
        AVFrame* convertedFrame;
        
        if (!frameBuffer.waitAndPop(frame))
        {
            // queue did close
            return;
        }
        //std::cout /*<< this << " "*/ << frameBuffer.size() << std::endl;
        
        if (!convertedFramePool.waitAndPop(convertedFrame))
        {
            // queue did close
            return;
        }
        
        if (!convertedFrame->data[0])
        {
            convertedFrame->width = frame->width;
            convertedFrame->height = frame->height;
            if (av_image_alloc(convertedFrame->data, convertedFrame->linesize, convertedFrame->width, convertedFrame->height,
                               (AVPixelFormat)convertedFrame->format, 32) < 0)
            {
                fprintf(stderr, "Could not allocate raw picture buffer\n");
                abort();
            }
        }
        
        if (frame->format != format)
        {
            // We have to convert the color format of this frame
            
            if (!imageConvertCtx)
            {
                // setup resizer for received frames
                imageConvertCtx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                                 convertedFrame->width, convertedFrame->height, format,
                                                 SWS_BICUBIC, NULL, NULL, NULL);
            }
            
            
            
            // resize frame
            sws_scale(imageConvertCtx, frame->data, frame->linesize, 0, frame->height,
                      convertedFrame->data, convertedFrame->linesize);
        }
        else
        {
            // We only have to copy the frame since the color format is already the desired one
            av_frame_copy(convertedFrame, frame);
        }
            
        
        
        convertedFrame->pts = frame->pts;
        convertedFrame->coded_picture_number = frame->coded_picture_number;
        
        if (onColorConvertedFrame) onColorConvertedFrame(this,
                                                         frame->key_frame,
                                                         avpicture_get_size((AVPixelFormat)frame->format,
                                                                            frame->width,
                                                                            frame->height));
        
        // continue decoding
        framePool.push(frame);
        
        // make frame available
        convertedFrameBuffer.push(convertedFrame);
		//convertedFramePool.push(convertedFrame);
    }
}

AVFrame* H264NALUSink::getNextFrame()
{
    AVFrame* frame;
	if (convertedFrameBuffer.tryPop(frame))
	{
        return frame;
	}
    else
    {
        return NULL;
    }
}

void H264NALUSink::returnFrame(AVFrame* frame)
{
	if (frame)
    {
		convertedFramePool.push(frame);
	}
}