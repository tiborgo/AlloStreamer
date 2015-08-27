#include <vector>

#include "H264CubemapSource.h"

void H264CubemapSource::setOnNextCubemap(std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)>& callback)
{
    onNextCubemap = callback;
}

void H264CubemapSource::setOnDroppedNALU(std::function<void (CubemapSource*, int face, u_int8_t type, size_t)>& callback)
{
    onDroppedNALU = callback;
}

void H264CubemapSource::setOnAddedNALU(std::function<void (CubemapSource*, int face, u_int8_t type, size_t)>& callback)
{
    onAddedNALU = callback;
}

void H264CubemapSource::getNextCubemapLoop()
{
	std::vector<AVFrame*> frames;

    while (true)
    {
        // Get all the decoded frames
        //std::vector<AVFrame*> nextFrames;
        //nextFrames.reserve(sinks.size());
        for (int i = 0; i < sinks.size(); i++)
        {
			while (frames.size() < i + 1)
			{
				frames.push_back(nullptr);
			}

			frames[i] = sinks[i]->getNextFrame(frames[i]);
        }

		StereoCubemap* cubemap;
		if (!oldCubemap)
		{
			std::vector<Cubemap*> eyes;
			for (int j = 0, faceIndex = 0; j < StereoCubemap::MAX_EYES_COUNT && faceIndex < sinks.size(); j++)
			{
				std::vector<CubemapFace*> faces;
				for (int i = 0; i < Cubemap::MAX_FACES_COUNT && faceIndex < sinks.size(); i++, faceIndex++)
				{
                    Frame* content = Frame::create(frames[i + j * Cubemap::MAX_FACES_COUNT]->width,
						                           frames[i + j * Cubemap::MAX_FACES_COUNT]->height,
						                           format,
						                           boost::chrono::system_clock::time_point(),
						                           heapAllocator);

					CubemapFace* face = CubemapFace::create(content,
						                                    i,
						                                    heapAllocator);

					faces.push_back(face);
				}

				
				eyes.push_back(Cubemap::create(faces, heapAllocator));
			}

			cubemap = StereoCubemap::create(eyes, heapAllocator);
		}
		else
		{
			cubemap = oldCubemap;
		}
        
		for (int j = 0, frameIndex = 0; j < cubemap->getEyesCount(); j++)
		{
			Cubemap* eye = cubemap->getEye(j);

			for (int i = 0; i < eye->getFacesCount(); i++, frameIndex++)
			{
				CubemapFace* face = eye->getFace(i);


				AVFrame* nextFrame = frames[frameIndex];
				if (nextFrame)
				{
					// read pixels from frame
					/*if (avpicture_layout((AVPicture*)nextFrame, (AVPixelFormat)nextFrame->format,
										 nextFrame->width, nextFrame->height,
										 (unsigned char*)face->getContent()->getPixels(), nextFrame->width * nextFrame->height * 4) < 0)
										 {
										 fprintf(stderr, "Could not read pixels from frame\n");
										 abort();
										 }*/
					memcpy(face->getContent()->getPixels(), nextFrame->data[0], nextFrame->width * nextFrame->height * 4);

					// delete nextFrame
					//av_freep(&nextFrame->data[0]);
				}
				else
				{
					// error
					std::cerr << "Cannot get next cubemap" << std::endl;
					abort();
				}
				//
				//        /*AVRational microSecBase = { 1, 1000000 };
				//        boost::chrono::microseconds presentationTimeSinceEpoch =
				//        boost::chrono::microseconds(av_rescale_q(nextFrame->pts, codecContext->time_base, microSecBase));*/



			}
            
        }

        
        
        if (onNextCubemap)
        {
            oldCubemap = onNextCubemap(this, cubemap);
		}
    }
}

H264CubemapSource::H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks, AVPixelFormat format)
    :
	sinks(sinks), format(format), oldCubemap(nullptr)
{
    av_log_set_level(AV_LOG_WARNING);
    for (H264RawPixelsSink* sink : sinks)
    {
        std::function<void (H264RawPixelsSink *, u_int8_t, size_t)> onDroppedNaluCallback =
            boost::bind(&H264CubemapSource::sinkOnDroppedNALU, this, _1, _2, _3);
        sink->setOnDroppedNALU(onDroppedNaluCallback);
        std::function<void (H264RawPixelsSink *, u_int8_t, size_t)> onAddedNaluCallback =
            boost::bind(&H264CubemapSource::sinkOnAddedNALU, this, _1, _2, _3);
        sink->setOnAddedNALU(onAddedNaluCallback);
    }
    getNextCubemapThread = boost::thread(boost::bind(&H264CubemapSource::getNextCubemapLoop, this));
}

void H264CubemapSource::sinkOnDroppedNALU(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = std::find(sinks.begin(), sinks.end(), sink) - sinks.begin();
    assert(face < sinks.size());
    onDroppedNALU(this, face, type, size);
}

void H264CubemapSource::sinkOnAddedNALU(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = std::find(sinks.begin(), sinks.end(), sink) - sinks.begin();
    assert(face < sinks.size());
    onAddedNALU(this, face, type, size);
}
