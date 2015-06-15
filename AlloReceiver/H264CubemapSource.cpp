#include "H264CubemapSource.h"

Stats stats;

void H264CubemapSource::setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>& onNextCubemap)
{
    this->onNextCubemap = onNextCubemap;
}

void H264CubemapSource::getNextCubemapLoop()
{
    while (true)
    {
        std::vector<CubemapFace*> faces;
        for (int i = 0; i < sinks.size(); i++)
        {
            CubemapFace* face = CubemapFace::create(resolution,
                                                    resolution,
                                                    i,
                                                    format,
                                                    boost::chrono::system_clock::time_point(),
                                                    heapAllocator);
            
            AVFrame* nextFrame = sinks[i]->getNextFrame();
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
            
                // read pixels from frame
                if (avpicture_layout((AVPicture*)resizedFrame, (AVPixelFormat)resizedFrame->format,
                                     resizedFrame->width, resizedFrame->height,
                                     (unsigned char*)face->getPixels(), resizedFrame->width * resizedFrame->height * 4) < 0)
                {
                    fprintf(stderr, "Could not read pixels from frame\n");
                    exit(0);
                }
                
                av_freep(&resizedFrame->data[0]);
                av_frame_free(&resizedFrame);
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
    getNextCubemapThread = boost::thread(boost::bind(&H264CubemapSource::getNextCubemapLoop, this));
}
