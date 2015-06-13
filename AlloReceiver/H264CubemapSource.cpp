#include "H264CubemapSource.h"

Stats stats;

const unsigned int SINK_BUFFER_SIZE = 2000000;

StereoCubemap* H264CubemapSource::getCurrentCubemap()
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
        if (nextFrame || lastFrames[i])
        {
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
                    return nullptr;
                }
                resizedFrame->format = format;
                resizedFrame->width = resolution;
                resizedFrame->height = resolution;

                if (av_image_alloc(resizedFrame->data, resizedFrame->linesize, resizedFrame->width, resizedFrame->height,
                                   (AVPixelFormat)resizedFrame->format, 32) < 0)
                {
                    fprintf(stderr, "Could not allocate raw picture buffer\n");
                    return nullptr;
                }

                // resize frame
                sws_scale(resizeCtx, nextFrame->data, nextFrame->linesize, 0, nextFrame->height,
                          resizedFrame->data, resizedFrame->linesize);

                // delete nextFrame
                av_freep(&nextFrame->data[0]);
                //av_frame_free(&nextFrame);

                // delete lastFrames[i]
                if (lastFrames[i])
                {
                    av_freep(&lastFrames[i]->data[0]);
                    av_frame_free(&lastFrames[i]);
                }
                
                lastFrames[i] = resizedFrame;
            }
        
            // read pixels from frame
            if (avpicture_layout((AVPicture*)lastFrames[i], (AVPixelFormat)lastFrames[i]->format,
                                 lastFrames[i]->width, lastFrames[i]->height,
                                 (unsigned char*)face->getPixels(), lastFrames[i]->width * lastFrames[i]->height * 4) < 0)
            {
                fprintf(stderr, "Could not read pixels from frame\n");
                return nullptr;
            }
        }
        else
        {
            // just use the default pixel buffer of the face
        }
//
//        /*AVRational microSecBase = { 1, 1000000 };
//        boost::chrono::microseconds presentationTimeSinceEpoch =
//        boost::chrono::microseconds(av_rescale_q(nextFrame->pts, codecContext->time_base, microSecBase));*/
        
        
        
        
        faces.push_back(face);
    }

    std::vector<Cubemap*> eyes;
    eyes.push_back(Cubemap::create(faces, heapAllocator));
    return StereoCubemap::create(eyes, heapAllocator);
    return nullptr;
}

H264CubemapSource::H264CubemapSource(const char* url, int resolution, AVPixelFormat format)
    :
    resizeCtx(NULL), resolution(resolution), format(format), didIdentifyStreamsBarrier(2)
{
    av_log_set_level(AV_LOG_WARNING);
    
    client = RTSPCubemapSourceClient::createNew(url, SINK_BUFFER_SIZE);
    client->delegate = this;
    client->connect();
    // wait until streams are identified
    didIdentifyStreamsBarrier.wait();
}

MediaSink* H264CubemapSource::getSinkForSubsession(RTSPCubemapSourceClient* client, MediaSubsession* subsession)
{
    static int counter = 0;
    
    if (strcmp(subsession->mediumName(), "video") == 0 &&
        strcmp(subsession->codecName(), "H264") == 0)
    {
        std::cout << "xxxxxxx " << counter++ << " " << subsession->savedSDPLines() << std::endl;
        H264RawPixelsSink* sink = H264RawPixelsSink::createNew(client->envir(), SINK_BUFFER_SIZE);
        sinks.push_back(sink);
        std::cout << "yyyyyyy " << sinks.size() << std::endl;
        lastFrames.push_back(NULL);
        return sink;
    }
    else
    {
        return NULL;
    }
}

void H264CubemapSource::didIdentifyStreams(RTSPCubemapSourceClient *client)
{
    didIdentifyStreamsBarrier.wait();
}
