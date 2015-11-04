#include <vector>
#include <unordered_map>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
extern "C"
{
    #include <libavcodec/avcodec.h>
}


#include "H264CubemapSource.h"

void H264CubemapSource::setOnReceivedNALU(const OnReceivedNALU& callback)
{
    onReceivedNALU = callback;
}

void H264CubemapSource::setOnReceivedFrame(const OnReceivedFrame& callback)
{
    onReceivedFrame = callback;
}

void H264CubemapSource::setOnDecodedFrame(const OnDecodedFrame& callback)
{
    onDecodedFrame = callback;
}

void H264CubemapSource::setOnColorConvertedFrame(const OnColorConvertedFrame& callback)
{
    onColorConvertedFrame = callback;
}

void H264CubemapSource::setOnNextCubemap(const OnNextCubemap& callback)
{
    onNextCubemap = callback;
}

void H264CubemapSource::setOnAddedFrameToCubemap(const OnAddedFrameToCubemap& callback)
{
    onAddedFrameToCubemap = callback;
}

void H264CubemapSource::setOnScheduledFrameInCubemap(const OnScheduledFrameInCubemap& callback)
{
    onScheduledFrameInCubemap = callback;
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
                
                int64_t key;
                if (robustSyncing)
                {
                    key = frames[i]->pts;
                }
                else
                {
                    key = frames[i]->coded_picture_number;
                }
                
                if (key <= lastFrameSeqNum)
                {
                    std::cout << "frame comes too late (" << lastFrameSeqNum-key+1 << " frame/s)" << std::endl;
                    sinks[i]->returnFrame(frames[i]);
                    continue;
                }
                
                if (frameMap.find(key) == frameMap.end())
                {
                    frameMap[key].resize(sinks.size());
                }
                
                std::vector<AVFrame*>& bucketFrames = frameMap[key];
                if (bucketFrames[i])
                {
                    // Matches should not happen here.
                    // If it happens give back frame immediately
                    std::cout << "match!? (" << i << ", " << key << ")" << std::endl;
                    sinks[i]->returnFrame(frames[i]);
                }
                else
                {
                    bucketFrames[i] = frames[i];
                    if (onAddedFrameToCubemap) onAddedFrameToCubemap(this, i);
                }
                
                if (frameMap.size() >= maxFrameMapSize)
                {
                    frameMapCondition.notify_all();
                }
            }
        }
    }
}

void H264CubemapSource::getNextCubemapLoop()
{
    int64_t lastPTS = 0;
    boost::chrono::system_clock::time_point lastDisplayTime(boost::chrono::microseconds(0));
    
    while (true)
    {
        size_t pendingCubemaps;
        int frameSeqNum;
        // Get frames with the oldest frame seq # and remove the associated bucket
        std::vector<AVFrame*> frames;
        {
            boost::mutex::scoped_lock lock(frameMapMutex);
            
            if (frameMap.size() < maxFrameMapSize)
            {
                frameMapCondition.wait(lock);
            }
            pendingCubemaps = frameMap.size();
            
            auto it = frameMap.begin();
            frameSeqNum = it->first;
            lastFrameSeqNum = frameSeqNum;
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
            AVFrame*     leftFrame = frames[i];
            CubemapFace* leftFace  = cubemap->getEye(0)->getFace(i, true);
            
            AVFrame*     rightFrame = nullptr;
            CubemapFace* rightFace  = nullptr;
            
            if (frames.size() > i + CUBEMAP_MAX_FACES_COUNT)
            {
                rightFrame = frames[i+CUBEMAP_MAX_FACES_COUNT];
                rightFace  = cubemap->getEye(1)->getFace(i, true);
            }
            
            if (matchStereoPairs && frames.size() > i + CUBEMAP_MAX_FACES_COUNT)
            {
                // check if matched
                if (!leftFrame || !rightFrame)
                {
                    // if they don't match give them back and forget about them
                    sinks[i]->returnFrame(leftFrame);
                    sinks[i + CUBEMAP_MAX_FACES_COUNT]->returnFrame(rightFrame);
                    leftFrame  = nullptr;
                    rightFrame = nullptr;
                }
            }
            
            // Fill the cubemapfaces with pixels if pixels are available
            if (leftFrame)
            {
                count++;
                leftFace->setNewFaceFlag(true);
                avpicture_layout((AVPicture*)leftFrame, (AVPixelFormat)leftFrame->format,
                                 leftFrame->width, leftFrame->height,
                                 (unsigned char*)leftFace->getContent()->getPixels(), leftFace->getContent()->getWidth() * leftFace->getContent()->getHeight() * 4);
                sinks[i]->returnFrame(leftFrame);
                if (onScheduledFrameInCubemap) onScheduledFrameInCubemap(this, i);
            }
            else
            {
                leftFace->setNewFaceFlag(false);
            }
            
            if (rightFrame)
            {
                count++;
                rightFace->setNewFaceFlag(true);
                avpicture_layout((AVPicture*)rightFrame, (AVPixelFormat)rightFrame->format,
                                 rightFrame->width, rightFrame->height,
                                 (unsigned char*)rightFace->getContent()->getPixels(), rightFace->getContent()->getWidth() * rightFace->getContent()->getHeight() * 4);
                sinks[i + CUBEMAP_MAX_FACES_COUNT]->returnFrame(rightFrame);
                if (onScheduledFrameInCubemap) onScheduledFrameInCubemap(this, i+CUBEMAP_MAX_FACES_COUNT);
            }
            else if (rightFace)
            {
                rightFace->setNewFaceFlag(false);
            }
        }
        
        // Give it to the user of this library (AlloPlayer etc.)
        if (onNextCubemap)
        {
            // calculate PTS for the cubemap (median of the individual faces' PTS)
            boost::accumulators::accumulator_set<int64_t, boost::accumulators::features<boost::accumulators::tag::median> > acc;
            for (AVFrame* frame : frames)
            {
                if (frame)
                {
                    acc(frame->pts);
                }
            }
            int64_t pts = boost::accumulators::median(acc);
            
            // Calculate time interval from until this cubemap should be displayed
            if (lastPTS == 0)
            {
                lastPTS = pts;
            }
            
            if (lastDisplayTime.time_since_epoch().count() == 0)
            {
                lastDisplayTime = boost::chrono::system_clock::now();
                lastDisplayTime += boost::chrono::seconds(5);
            }
            uint64_t ptsDiff = pts - lastPTS;
            lastDisplayTime += boost::chrono::microseconds(ptsDiff);
            lastPTS = pts;
            
            boost::chrono::milliseconds sleepDuration = boost::chrono::duration_cast<boost::chrono::milliseconds>(lastDisplayTime - boost::chrono::system_clock::now());
            
            // Wait until frame should be displayed
            //boost::this_thread::sleep_for(sleepDuration);
            
            // Display frame
            oldCubemap = onNextCubemap(this, cubemap);
		}
    }
}

H264CubemapSource::H264CubemapSource(std::vector<H264RawPixelsSink*>& sinks,
                                     AVPixelFormat                    format,
                                     bool                             matchStereoPairs,
                                     bool                             robustSyncing,
                                     size_t                           maxFrameMapSize)
    :
    sinks(sinks), format(format), oldCubemap(nullptr), lastFrameSeqNum(0), matchStereoPairs(matchStereoPairs),
    robustSyncing(robustSyncing), maxFrameMapSize(maxFrameMapSize)
{
    int i = 0;
    for (H264RawPixelsSink* sink : sinks)
    {
        sink->setOnReceivedNALU       (boost::bind(&H264CubemapSource::sinkOnReceivedNALU,        this, _1, _2, _3));
        sink->setOnReceivedFrame      (boost::bind(&H264CubemapSource::sinkOnReceivedFrame,       this, _1, _2, _3));
        sink->setOnDecodedFrame       (boost::bind(&H264CubemapSource::sinkOnDecodedFrame,        this, _1, _2, _3));
        sink->setOnColorConvertedFrame(boost::bind(&H264CubemapSource::sinkOnColorConvertedFrame, this, _1, _2, _3));
        
        sinksFaceMap[sink] = i;
        i++;
    }
    getNextFramesThread  = boost::thread(boost::bind(&H264CubemapSource::getNextFramesLoop,  this));
    getNextCubemapThread = boost::thread(boost::bind(&H264CubemapSource::getNextCubemapLoop, this));
}

void H264CubemapSource::sinkOnReceivedNALU(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = sinksFaceMap[sink];
    if (onReceivedNALU) onReceivedNALU(this, type, size, face);
}

void H264CubemapSource::sinkOnReceivedFrame(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = sinksFaceMap[sink];
    if (onReceivedFrame) onReceivedFrame(this, type, size, face);
}

void H264CubemapSource::sinkOnDecodedFrame(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = sinksFaceMap[sink];
    if (onDecodedFrame) onDecodedFrame(this, type, size, face);
}

void H264CubemapSource::sinkOnColorConvertedFrame(H264RawPixelsSink* sink, u_int8_t type, size_t size)
{
    int face = sinksFaceMap[sink];
    if (onColorConvertedFrame) onColorConvertedFrame(this, type, size, face);
}

