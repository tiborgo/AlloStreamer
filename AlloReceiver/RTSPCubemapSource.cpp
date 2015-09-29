#include "RTSPCubemapSource.hpp"
#include "H264CubemapSource.h"

struct RTSPHelper
{
    RTSPCubemapSource* cubemapSource;
    AVPixelFormat      format;
    boost::barrier     didCreateCubemapSource;
	unsigned long      bufferSize;
    
    RTSPHelper() : didCreateCubemapSource(2) {}
    
    std::vector<MediaSink*> onGetSinksForSubsessions(RTSPCubemapSourceClient* client,
                                                     std::vector<MediaSubsession*>& subsessions)
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

		if (!isH264)
		{
			abort();
		}

        std::vector<H264RawPixelsSink*> h264Sinks;
        std::vector<MediaSink*> sinks;
        for (int i = 0; i < (std::min)(subsessions.size(), (size_t)(StereoCubemap::MAX_EYES_COUNT * Cubemap::MAX_FACES_COUNT)); i++)
        {
            H264RawPixelsSink* sink = H264RawPixelsSink::createNew(client->envir(),
						                                           bufferSize,
                                                                   format,
                                                                   subsessions[i]);
            h264Sinks.push_back(sink);
            sinks.push_back(sink);
        }
                
        cubemapSource = new H264CubemapSource(h264Sinks, format);
        didCreateCubemapSource.wait();
        return sinks;
    }
};

void RTSPCubemapSource::setOnNextCubemap(std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)>& callback)
{
    onNextCubemap = callback;
}

void RTSPCubemapSource::setOnDroppedNALU(std::function<void (CubemapSource*, int face, u_int8_t type, size_t)>& callback)
{
    onDroppedNALU = callback;
}

void RTSPCubemapSource::setOnAddedNALU(std::function<void (CubemapSource*, int face, u_int8_t type, size_t)>& callback)
{
    onAddedNALU = callback;
}

RTSPCubemapSource* RTSPCubemapSource::createFromRTSP(const char*   url,
	                                                 unsigned long bufferSize,
                                                     AVPixelFormat format,
                                                     const char*   interfaceAddress)
{
    NetAddressList addresses(interfaceAddress);
    if (addresses.numAddresses() == 0)
    {
       std::cout << "Inteface \"" << interfaceAddress << "\" does not exist" << std::endl;
       return nullptr;
    }
    ReceivingInterfaceAddr = *(unsigned*)(addresses.firstAddress()->data());

    //RTSPCubemapSourceClient* client = RTSPCubemapSourceClient::createNew(url, bufferSize);
    RTSPHelper helper;
    helper.format = format;
	helper.bufferSize = bufferSize;
    client->onGetSinksForSubsessions = boost::bind(&RTSPHelper::onGetSinksForSubsessions, &helper, _1, _2);
    client->connect();
    // wait until streams are identified
    helper.didCreateCubemapSource.wait();
    return new RTSPCubemapSource(client, helper.cubemapSource);
}

const RTSPCubemapSourceClient* RTSPCubemapSource::getClient()
{
    return client;
}
