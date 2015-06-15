#include "RTSPCubemapSource.hpp"
#include "H264CubemapSource.h"

const unsigned int SINK_BUFFER_SIZE = 2000000;

struct RTSPHelper
{
    RTSPCubemapSource* cubemapSource;
    int                resolution;
    AVPixelFormat      format;
    boost::barrier     didCreateCubemapSource;
    
    RTSPHelper() : didCreateCubemapSource(2) {}
    
    std::vector<MediaSink*> onGetSinksForSubsessions(RTSPCubemapSourceClient* client,
                                                     std::vector<MediaSubsession*>& subsessions)
    {
        if (subsessions.size() <= 6)
        {
            bool isH264 = true;
            for (MediaSubsession* subsession : subsessions)
            {
                if (strcmp(subsession->mediumName(), "video") != 0 ||
                    strcmp(subsession->codecName(), "H264") != 0)
                {
                    isH264 = false;
                }
            }
            
            if (isH264)
            {
                std::vector<H264RawPixelsSink*> h264Sinks;
                std::vector<MediaSink*> sinks;
                for (int i = 0; i < 6; i++)
                {
                    H264RawPixelsSink* sink = H264RawPixelsSink::createNew(client->envir(), SINK_BUFFER_SIZE);
                    h264Sinks.push_back(sink);
                    sinks.push_back(sink);
                }
                
                cubemapSource = new H264CubemapSource(h264Sinks, resolution, format);
                didCreateCubemapSource.wait();
                return sinks;
            }
        }
        
        cubemapSource = nullptr;
        didCreateCubemapSource.wait();
        return std::vector<MediaSink*>();
    }
};

RTSPCubemapSource* RTSPCubemapSource::create(const char* url,
                                             int resolution,
                                             AVPixelFormat format)
{
    RTSPCubemapSourceClient* client = RTSPCubemapSourceClient::createNew(url, SINK_BUFFER_SIZE);
    RTSPHelper helper;
    helper.resolution = resolution;
    helper.format = format;
    client->onGetSinksForSubsessions = boost::bind(&RTSPHelper::onGetSinksForSubsessions, &helper, _1, _2);
    client->connect();
    // wait until streams are identified
    helper.didCreateCubemapSource.wait();
    return helper.cubemapSource;
}
