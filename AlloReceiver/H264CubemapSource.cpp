#include <vector>
#include <unordered_map>
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

void H264CubemapSource::getNextFramesLoop()
{
	std::vector<AVFrame*> frames(sinks.size(), nullptr);
	
    while (true)
    {
        // Get all the decoded frames
        for (int i = 0; i < sinks.size(); i++)
        {
			frames[i] = sinks[i]->getNextFrame();
            
            if (frames[i])
            {
                boost::mutex::scoped_lock lock(frameMapMutex);
                
                // The pts get changed a little by live555.
                // Find a pts that is close enough so that we can put it in the container
                // with the right other faces
                uint64_t key = 0;
                for (auto it = frameMap.begin(); it != frameMap.end(); ++it)
                {
                    if (std::abs(it->first - frames[i]->pts) < 3000)
                    {
                        key = it->first;
                        break;
                    }
                }
                
                if (key == 0)
                {
                    key = frames[i]->pts;
                    frameMap[key].resize(sinks.size());
                }
                
                std::vector<AVFrame*>& bucketFrames = frameMap[key];
                if (bucketFrames[i])
                {
                    // Matches should not happen here.
                    // If it happens give back frame immediately
                    //std::cout << "match!? (" << i << ")" << bucketFrames[i]->pts << " " << frames[i]->pts << std::endl;
                    sinks[i]->returnFrame(frames[i]);
                }
                else
                {
                    bucketFrames[i] = frames[i];
                    //std::cout << "matched bucket " << frames[i] << " " << key << " " << frames[i]->pts << std::endl;
                }
            }
        }
        //std::cout << frameMap.size() << std::endl;
    }
}

void H264CubemapSource::getNextCubemapLoop()
{
    while (true)
    {
        // Get frames with the oldest pts and remove the associated bucket
        std::vector<AVFrame*> frames;
        {
            boost::mutex::scoped_lock lock(frameMapMutex);
            
            if (frameMap.size() < 10)
            {
                continue;
            }
            
            std::map<int64_t, std::vector<AVFrame*>>::iterator it = frameMap.begin();
            frames = it->second;
            frameMap.erase(it);
        }
        
        StereoCubemap* cubemap;
        
        // Allocate cubemap if necessary
        if (!oldCubemap)
        {
            int width, height;
            for (AVFrame* frame : frames)
            {
                if (frame)
                {
                    width  = frame->width;
                    height = frame->height;
                    break;
                }
            }
            
            std::vector<Cubemap*> eyes;
            for (int j = 0, faceIndex = 0; j < StereoCubemap::MAX_EYES_COUNT && faceIndex < sinks.size(); j++)
            {
                std::vector<CubemapFace*> faces;
                for (int i = 0; i < Cubemap::MAX_FACES_COUNT && faceIndex < sinks.size(); i++, faceIndex++)
                {
                    Frame* content = Frame::create(width,
                                                   height,
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
        
        size_t count = 0;
        // Fill cubemap making sure stereo pairs match
        for (int i = 0; i < (std::min)(frames.size(), (size_t)CUBEMAP_MAX_FACES_COUNT); i++)
        {
            AVFrame* leftFrame = frames[i];
            Cubemap* leftEye  = cubemap->getEye(0);
            CubemapFace* leftFace  = leftEye->getFace(i, true);
            
            if (frames.size() > i + CUBEMAP_MAX_FACES_COUNT)
            {
                AVFrame* rightFrame = frames[i+CUBEMAP_MAX_FACES_COUNT];
                
                Cubemap* rightEye = cubemap->getEye(1);
                CubemapFace* rightFace = rightEye->getFace(i, true);
                
                // check if matched
                if (leftFrame && rightFrame)
                {
                    count++;
                    leftFace->setNewFaceFlag(true);
                    memcpy(leftFace->getContent()->getPixels(), leftFrame->data[0], leftFrame->width * leftFrame->height * 4);
                    
                    count++;
                    rightFace->setNewFaceFlag(true);
                    memcpy(rightFace->getContent()->getPixels(), rightFrame->data[0], rightFrame->width * rightFrame->height * 4);
                }
                else
                {
                    leftFace->setNewFaceFlag(false);
                    rightFace->setNewFaceFlag(false);
                }
                
                sinks[i + CUBEMAP_MAX_FACES_COUNT]->returnFrame(rightFrame);
            }
            else
            {
                // matching not possible -> take the frame
                if (leftFrame)
                {
                    count++;
                    leftFace->setNewFaceFlag(true);
                    memcpy(leftFace->getContent()->getPixels(), leftFrame->data[0], leftFrame->width * leftFrame->height * 4);
                }
                else
                {
                    leftFace->setNewFaceFlag(false);
                }
            }
            
            sinks[i]->returnFrame(leftFrame);
        }

        //std::cout << count << std::endl;
        
        // Give it to the user of this library (AlloPlayer etc.)
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
    getNextFramesThread  = boost::thread(boost::bind(&H264CubemapSource::getNextFramesLoop,  this));
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
