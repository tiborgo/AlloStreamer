#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <liveMedia.hh>
#include <GroupsockHelper.hh>
#define EventTime server_EventTime
#include <BasicUsageEnvironment.hh>
#undef EventTime

extern "C"
{
    #include <libavcodec/avcodec.h>
}

#include "AlloShared/to_human_readable_byte_count.hpp"
#include "AlloShared/concurrent_queue.h"
#include "AlloShared/Cubemap.hpp"
#include "AlloShared/Binoculars.hpp"
#include "AlloShared/config.h"
#include "AlloShared/Process.h"
#include "AlloShared/StatsUtils.hpp"
#include "config.h"
#include "RawPixelSource.hpp"
#include "CubemapExtractionPlugin/CubemapExtractionPlugin.h"
#include "AlloServer.h"
#include "AlloReceiver/Stats.hpp"

static Stats stats;

struct FrameStreamState
{
    RTPSink*      sink;
    Frame*        content;
    FramedSource* source;
};

static UsageEnvironment* env;
static struct in_addr destinationAddress;
static RTSPServer* rtspServer;
static char const* descriptionString
    = "Session streamed by \"AlloUnity\"";
static boost::interprocess::managed_shared_memory* shm;
static boost::barrier stopStreamingBarrier(3);
static boost::uint16_t rtspPort;
static int avgBitRate;

static size_t bufferSize = 2000000000;
static bool robustSyncing = false;

// Cubemap related
static StereoCubemap*                cubemap;
static ServerMediaSession*           cubemapSMS;
static EventTriggerId                addFaceSubstreamsTriggerId;
static EventTriggerId                removeFaceSubstreamsTriggerId;
static std::string                   cubemapStreamName = "cubemap";
static std::vector<FrameStreamState> faceStreams;

// Binoculars related
static Binoculars* binoculars = nullptr;
static ServerMediaSession* binocularsSMS = NULL;
static EventTriggerId addBinularsSubstreamTriggerId;
static EventTriggerId removeBinularsSubstreamTriggerId;
static std::string binocularsStreamName = "binoculars";
static FrameStreamState* binocularsStream = nullptr;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, std::string& name)
{
    char* url = rtspServer->rtspURL(sms);
    std::cout << "Play " << name << " using the URL \"" << url << "\"" << std::endl;
    delete[] url;
}

void onSentNALU(RawPixelSource*, uint8_t type, size_t size, int eye, int face)
{
	//stats.store(StatsUtils::NALU(type, size, eye * 6 + face, StatsUtils::NALU::SENT));
}

void onEncodedFrame(RawPixelSource*, int eye, int face)
{
	stats.store(StatsUtils::CubemapFace(eye * 6 + face, StatsUtils::CubemapFace::DISPLAYED));
}

void addFaceSubstreams0(void*)
{
	int portCounter = 0;
	for (int j = 0; j < cubemap->getEyesCount(); j++)
	{
		Cubemap* eye = cubemap->getEye(j);

		for (int i = 0; i < eye->getFacesCount(); i++)
		{
			faceStreams.push_back(FrameStreamState());
			FrameStreamState* state = &faceStreams.back();

			state->content = eye->getFace(i)->getContent();

			Port rtpPort(FACE0_RTP_PORT_NUM + portCounter);
			portCounter += 2;
			Groupsock* rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPort, TTL);
			//rtpGroupsock->multicastSendOnly(); // we're a SSM source

			setReceiveBufferTo(*env, rtpGroupsock->socketNum(), bufferSize);

			// Create a 'H264 Video RTP' sink from the RTP 'groupsock':
			state->sink = H264VideoRTPSink::createNew(*env, rtpGroupsock, 96);

			ServerMediaSubsession* subsession = PassiveServerMediaSubsession::createNew(*state->sink);

			cubemapSMS->addSubsession(subsession);

			RawPixelSource* source = RawPixelSource::createNew(*env,
				state->content,
				avgBitRate,
				robustSyncing);

			source->setOnSentNALU    (boost::bind(&onSentNALU,     _1, _2, _3, j, i));
			source->setOnEncodedFrame(boost::bind(&onEncodedFrame, _1, j, i));

			state->source = H264VideoStreamDiscreteFramer::createNew(*env,
				source);

			state->sink->startPlaying(*state->source, NULL, NULL);

			std::cout << "Streaming face " << i << " (" << ((j == 0) ? "left" : "right") << ") on port " << ntohs(rtpPort.num()) << " ..." << std::endl;
		}
	}
    
    announceStream(rtspServer, cubemapSMS, cubemapStreamName);
}

void removeFaceSubstreams0(void*)
{
    if (faceStreams.size() > 0)
    {
        rtspServer->closeAllClientSessionsForServerMediaSession(cubemapSMS);
        for (int i = 0; i < faceStreams.size(); i++)
        {
            FrameStreamState stream = faceStreams[i];
            stream.sink->stopPlaying();
            Medium::close(stream.sink);
            Medium::close(stream.source);
            std::cout << "removed face " << i << std::endl;
        }
        faceStreams.clear();
        cubemapSMS->deleteAllSubsessions();
    }
    boost::thread(boost::bind(&boost::barrier::wait, &stopStreamingBarrier));
}

void addBinocularsSubstream0(void*)
{
    binocularsStream = new FrameStreamState;
    binocularsStream->content = binoculars->getContent();
    
    Port rtpPort(BINOCULARS_RTP_PORT_NUM);
    Groupsock* rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPort, TTL);
    //rtpGroupsock->multicastSendOnly(); // we're a SSM source
    
    // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
    binocularsStream->sink = H264VideoRTPSink::createNew(*env, rtpGroupsock, 96);
    
    ServerMediaSubsession* subsession = PassiveServerMediaSubsession::createNew(*binocularsStream->sink);
    
    binocularsSMS->addSubsession(subsession);
    
    binocularsStream->source = H264VideoStreamDiscreteFramer::createNew(*env,
                                                                        RawPixelSource::createNew(*env,
                                                                                                  binocularsStream->content,
                                                                                                  avgBitRate,
																								  robustSyncing));
    binocularsStream->sink->startPlaying(*binocularsStream->source, NULL, NULL);
    
    std::cout << "Streaming binoculars ..." << std::endl;
    
    announceStream(rtspServer, binocularsSMS, binocularsStreamName);
}

void removeBinocularsSubstream0(void*)
{
    if (binocularsStream)
    {
        rtspServer->closeAllClientSessionsForServerMediaSession(binocularsSMS);

        binocularsStream->sink->stopPlaying();
        Medium::close(binocularsStream->sink);
        Medium::close(binocularsStream->source);
        std::cout << "removed binoculars" << std::endl;
        
        binocularsSMS->deleteAllSubsessions();
        delete binocularsStream;
    }
    boost::thread(boost::bind(&boost::barrier::wait, &stopStreamingBarrier));
}

void networkLoop()
{
    env->taskScheduler().doEventLoop(); // does not return
}

void setupRTSP()
{
	OutPacketBuffer::maxSize = bufferSize;

    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();

    env = BasicUsageEnvironment::createNew(*scheduler);
    
    char multicastAddressStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(destinationAddress.s_addr), multicastAddressStr, sizeof(multicastAddressStr));
    printf("Multicast address: %s\n", multicastAddressStr);


    // Create the RTSP server:
    rtspServer = RTSPServer::createNew(*env, rtspPort, NULL);

    if (rtspServer == NULL)
    {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    // Set up each of the possible streams that can be served by the
    // RTSP server.  Each such stream is implemented using a
    // "ServerMediaSession" object, plus one or more
    // "ServerMediaSubsession" objects for each audio/video substream.


	
    
    cubemapSMS = ServerMediaSession::createNew(*env,
                                               cubemapStreamName.c_str(),
                                               cubemapStreamName.c_str(),
                                               descriptionString,
                                               True);
    rtspServer->addServerMediaSession(cubemapSMS);
    
    binocularsSMS = ServerMediaSession::createNew(*env,
                                                  binocularsStreamName.c_str(),
                                                  binocularsStreamName.c_str(),
                                                  descriptionString,
                                                  True);
    rtspServer->addServerMediaSession(binocularsSMS);

    addFaceSubstreamsTriggerId = env->taskScheduler().createEventTrigger(&addFaceSubstreams0);
    removeFaceSubstreamsTriggerId = env->taskScheduler().createEventTrigger(&removeFaceSubstreams0);
    addBinularsSubstreamTriggerId = env->taskScheduler().createEventTrigger(&addBinocularsSubstream0);
    removeBinularsSubstreamTriggerId = env->taskScheduler().createEventTrigger(&removeBinocularsSubstream0);
}

void startStreaming()
{
    // Open already created shared memory object.
    // Must have read and write access since we are using mutexes
    // and locking a mutex is a write operation
    shm = new boost::interprocess::managed_shared_memory(boost::interprocess::open_only,
                                                         SHM_NAME);

    auto cubemapPair = shm->find<StereoCubemap::Ptr>("Cubemap");
    if (cubemapPair.first)
    {
        cubemap = cubemapPair.first->get();
        env->taskScheduler().triggerEvent(addFaceSubstreamsTriggerId, NULL);
    }
    else
    {
        cubemap = nullptr;
    }
    
    auto binocularsPair = shm->find<Binoculars::Ptr>("Binoculars");
    if (binocularsPair.first)
    {
        binoculars = binocularsPair.first->get();
        env->taskScheduler().triggerEvent(addBinularsSubstreamTriggerId, NULL);
    }
    else
    {
        binoculars = nullptr;
    }
}

void stopStreaming()
{

    env->taskScheduler().triggerEvent(removeFaceSubstreamsTriggerId, NULL);
    env->taskScheduler().triggerEvent(removeBinularsSubstreamTriggerId, NULL);
    stopStreamingBarrier.wait();
    
    delete shm;
}

int main(int argc, char* argv[])
{
    boost::program_options::options_description desc("");
	desc.add_options()
		("multicast-address", boost::program_options::value<std::string>(),     "")
		("interface",         boost::program_options::value<std::string>(),     "")
		("rtsp-port",         boost::program_options::value<boost::uint16_t>(), "")
		("avg-bit-rate",      boost::program_options::value<int>(),             "")
		("buffer-size",       boost::program_options::value<size_t>(),          "")
	    ("stats-interval",    boost::program_options::value<size_t>(),          "")
		("robust-syncing",    "");
		
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("interface"))
    {
        std::string interfaceAddress = vm["interface"].as<std::string>();
        NetAddressList addresses(interfaceAddress.c_str());
        if (addresses.numAddresses() == 0)
        {
            std::cout << "Failed to find network address for \"" << interfaceAddress << "\"" << std::endl;
            return -1;
        }
        ReceivingInterfaceAddr = *(unsigned*)(addresses.firstAddress()->data());
    }
    
    char sourceAddressStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ReceivingInterfaceAddr), sourceAddressStr, sizeof(sourceAddressStr));
    std::cout << "Using source address " << sourceAddressStr << std::endl;
    
    if (vm.count("rtsp-port"))
    {
        rtspPort = vm["rtsp-port"].as<boost::uint16_t>();
    }
    else
    {
        rtspPort = 8555;
    }
    std::cout << "Using RTSP port " << rtspPort << std::endl;
    
    // Create 'groupsocks' for RTP and RTCP:
    if(vm.count("multicast-address"))
    {
        inet_pton(AF_INET, vm["multicast-address"].as<std::string>().c_str(), &(destinationAddress.s_addr));
    }
    else
    {
        inet_pton(AF_INET, "224.0.67.67", &(destinationAddress.s_addr));
    }

	if (vm.count("avg-bit-rate"))
	{
		avgBitRate = vm["avg-bit-rate"].as<int>();
	}
	else
	{
		avgBitRate = DEFAULT_AVG_BIT_RATE;
	}
    std::string bitRateString = to_human_readable_byte_count(avgBitRate, true, false);
	std::cout << "Using an average encoding bit rate of " << bitRateString << "/s per face" << std::endl;

	if (vm.count("buffer-size"))
	{
		bufferSize = vm["buffer-size"].as<size_t>();
	}
	else
	{
		bufferSize = DEFAULT_BUFFER_SIZE;
	}
	std::string bufferSizeString = to_human_readable_byte_count(bufferSize, false, false);
	std::cout << "Using a buffer size of " << bufferSizeString << std::endl;

	size_t statsInterval;
	if (vm.count("stats-interval"))
	{
		statsInterval = vm["stats-interval"].as<size_t>();
	}
	else
	{
		statsInterval = DEFAULT_STATS_INTERVAL;
	}

	if (vm.count("robust-syncing"))
	{
		robustSyncing = true;
	}

    av_log_set_level(AV_LOG_WARNING);
    avcodec_register_all();
    setupRTSP();
    boost::thread networkThread = boost::thread(&networkLoop);

	

    Process unityProcess(CUBEMAPEXTRACTIONPLUGIN_ID, false);
	Process thisProcess(ALLOSERVER_ID, true);

    while (true)
    {
        std::cout << "Waiting for Unity ..." << std::endl;
        unityProcess.waitForBirth();
        std::cout << "Connected to Unity :)" << std::endl;
        startStreaming();
		stats.autoSummary(boost::chrono::seconds(statsInterval),
			              AlloReceiver::statValsMaker,
						  AlloReceiver::postProcessorMaker,
						  AlloReceiver::formatStringMaker());
        unityProcess.join();
        std::cout << "Lost connection to Unity :(" << std::endl;
        stopStreaming();
		stats.stopAutoSummary();
    }

    return 0;
}
