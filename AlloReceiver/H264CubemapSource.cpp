#include "H264CubemapSource.h"

void H264CubemapSource::setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>& callback)
{
    onNextCubemap = callback;
}

void H264CubemapSource::setOnDroppedNALU(std::function<void (CubemapSource*, int face, u_int8_t type)>& callback)
{
    onDroppedNALU = callback;
}

void H264CubemapSource::setOnAddedNALU(std::function<void (CubemapSource*, int face, u_int8_t type)>& callback)
{
    onAddedNALU = callback;
}

void H264CubemapSource::getNextCubemapLoop()
{
    while (true)
    {
        // Get all the decoded frames
        std::vector<AVFrame*> nextFrames;
        nextFrames.reserve(sinks.size());
        for (int i = 0; i < sinks.size(); i++)
        {
            nextFrames.push_back(sinks[i]->getNextFrame());
        }
        
        std::vector<CubemapFace*> faces;
        for (int i = 0; i < sinks.size(); i++)
        {
            CubemapFace* face = CubemapFace::create(resolution,
                                                    resolution,
                                                    i,
                                                    format,
                                                    boost::chrono::system_clock::time_point(),
                                                    heapAllocator);
            
            
            AVFrame* nextFrame = nextFrames[i];
            if (nextFrame)
            {
                if (!resizeCtx)
                {
                    // setup resizer for received frames
                    resizeCtx = sws_getContext(nextFrame->width, nextFrame->height, (AVPixelFormat)nextFrame->format,
                                               resolution, resolution, format,
                                               SWS_BICUBIC, NULL, NULL, NULL);
                }
                
                AVFrame* resizedFrame = av_frame_alloc();

                if (!resizedFrame)
                {
                    fprintf(stderr, "Could not allocate video frame\n");
                    exit(-1);
                }
                resizedFrame->format = format;
                resizedFrame->width = resolution;
                resizedFrame->height = resolution;

                if (av_image_alloc(resizedFrame->data, resizedFrame->linesize, resizedFrame->width, resizedFrame->height,
                                   (AVPixelFormat)resizedFrame->format, 32) < 0)
                {
                    fprintf(stderr, "Could not allocate raw picture buffer\n");
                    exit(-1);
                }

                // resize frame
                sws_scale(resizeCtx, nextFrame->data, nextFrame->linesize, 0, nextFrame->height,
                          resizedFrame->data, resizedFrame->linesize);

                // delete nextFrame
                av_freep(&nextFrame->data[0]);
                //av_frame_free(&nextFrame);
//                AVFrame* resizedFrame = nextFrame;
            
                // read pixels from frame
                if (avpicture_layout((AVPicture*)resizedFrame, (AVPixelFormat)resizedFrame->format,
                                     resizedFrame->width, resizedFrame->height,
                                     (unsigned char*)face->getPixels(), resizedFrame->width * resizedFrame->height * 4) < 0)
                {
                    fprintf(stderr, "Could not read pixels from frame\n");
                    exit(0);
                }
                
                av_freep(&resizedFrame->data[0]);
                //av_frame_free(&resizedFrame);
            }
            else
            {
                // error
                std::cerr << "Cannot get next cubemap" << std::endl;
                exit(-1);
            }
    //
    //        /*AVRational microSecBase = { 1, 1000000 };
    //        boost::chrono::microseconds presentationTimeSinceEpoch =
    //        boost::chrono::microseconds(av_rescale_q(nextFrame->pts, codecContext->time_base, microSecBase));*/
            
            
            
            
            faces.push_back(face);
        }

        std::vector<Cubemap*> eyes;
        eyes.push_back(Cubemap::create(faces, heapAllocator));
        StereoCubemap* cubemap = StereoCubemap::create(eyes, heapAllocator);
        
        if (onNextCubemap)
        {
            onNextCubemap(this, cubemap);
        }
    }
}

H264CubemapSource::H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks, int resolution, AVPixelFormat format)
    :
    sinks(sinks), resizeCtx(NULL), resolution(resolution), format(format)
{
    av_log_set_level(AV_LOG_WARNING);
    for (H264RawPixelsSink* sink : sinks)
    {
        std::function<void (H264RawPixelsSink *, u_int8_t)> onDroppedNaluCallback =
            boost::bind(&H264CubemapSource::sinkOnDroppedNALU, this, _1, _2);
        sink->setOnDroppedNALU(onDroppedNaluCallback);
        std::function<void (H264RawPixelsSink *, u_int8_t)> onAddedNaluCallback =
            boost::bind(&H264CubemapSource::sinkOnAddedNALU, this, _1, _2);
        sink->setOnAddedNALU(onAddedNaluCallback);
    }
    getNextCubemapThread = boost::thread(boost::bind(&H264CubemapSource::getNextCubemapLoop, this));
}

void H264CubemapSource::sinkOnDroppedNALU(H264RawPixelsSink* sink, u_int8_t type)
{
    int face = std::find(sinks.begin(), sinks.end(), sink) - sinks.begin();
    assert(face < sinks.size());
    onDroppedNALU(this, face, type);
}

void H264CubemapSource::sinkOnAddedNALU(H264RawPixelsSink* sink, u_int8_t type)
{
    int face = std::find(sinks.begin(), sinks.end(), sink) - sinks.begin();
    assert(face < sinks.size());
    onAddedNALU(this, face, type);
}
