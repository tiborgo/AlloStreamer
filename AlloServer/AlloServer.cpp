#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/filesystem/path.hpp>
#include <liveMedia.hh>
#include <GroupsockHelper.hh>
#define EventTime server_EventTime
#include <BasicUsageEnvironment.hh>
#undef EventTime

extern "C"
{
    #include <libavcodec/avcodec.h>
}

#include "AlloShared/concurrent_queue.h"
#include "AlloShared/Cubemap.hpp"
#include "AlloShared/config.h"
#include "AlloShared/Process.h"
#include "config.h"
#include "CubemapFaceSource.h"
#include "CubemapExtractionPlugin/CubemapExtractionPlugin.h"
#include "AlloServer.h"

struct FaceStreamState
{
    RTPSink* sink;
    CubemapFace* face;
    FramedSource* source;
};

static UsageEnvironment* env;
static ServerMediaSession* sms;
static EventTriggerId addFaceSubstreamsTriggerId;
static EventTriggerId removeFaceSubstreamsTriggerId;
static struct in_addr destinationAddress;
static RTSPServer* rtspServer;
static char const* streamName = "h264ESVideoTest";
static char const* descriptionString
    = "Session streamed by \"testOnDemandRTSPServer\"";
static boost::interprocess::managed_shared_memory* shm;
static boost::barrier stopStreamingBarrier(2);
static std::vector<FaceStreamState> faceStreams;
static std::atomic_bool isStoppingStreaming(false);

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms)
{
    char* url = rtspServer->rtspURL(sms);
    UsageEnvironment& env = rtspServer->envir();

    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

void addFaceSubstream0(void*)
{
    if (!sms)
    {
        sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                            descriptionString, True);
        rtspServer->addServerMediaSession(sms);
        announceStream(rtspServer, sms);
    }

    for (int i = 0; i < cubemap->getFacesCount(); i++)
    {
        faceStreams.push_back(FaceStreamState());
        FaceStreamState* state = &faceStreams.back();
        
        state->face = cubemap->getFace(i);

        Port rtpPort(RTP_PORT_NUM + state->face->getIndex());
        Groupsock* rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPort, TTL);
        rtpGroupsock->multicastSendOnly(); // we're a SSM source

        // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
        // OutPacketBuffer::maxSize = 100000;
        state->sink = H264VideoRTPSink::createNew(*env, rtpGroupsock, 96);

        ServerMediaSubsession* subsession = PassiveServerMediaSubsession::createNew(*state->sink);

        sms->addSubsession(subsession);

        state->source =  H264VideoStreamDiscreteFramer::createNew(*env, CubemapFaceSource::createNew(*env, state->face));
        state->sink->startPlaying(*state->source, NULL, NULL);

        std::cout << "added face " << state->face->getIndex() << std::endl;
    }
}

void removeFaceSubstreams0(void*)
{
    rtspServer->closeAllClientSessionsForServerMediaSession(sms);
    for (FaceStreamState faceStream : faceStreams)
    {
        faceStream.sink->stopPlaying();
        Medium::close(faceStream.sink);
        Medium::close(faceStream.source);
        std::cout << "removed face " << faceStream.face->getIndex() << std::endl;
    }
    faceStreams.clear();
    sms->deleteAllSubsessions();
    stopStreamingBarrier.wait();
}

void networkLoop()
{
    env->taskScheduler().doEventLoop(); // does not return
}

void setupRTSP(int rtspPort)
{
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();

    env = BasicUsageEnvironment::createNew(*scheduler);

    // Create 'groupsocks' for RTP and RTCP:
    destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
    // Note: This is a multicast address.  If you wish instead to stream
    // using unicast, then you should use the "testOnDemandRTSPServer"
    // test program - not this test program - as a model.

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


    OutPacketBuffer::maxSize = 4000000;

    addFaceSubstreamsTriggerId = env->taskScheduler().createEventTrigger(&addFaceSubstream0);
    removeFaceSubstreamsTriggerId = env->taskScheduler().createEventTrigger(&removeFaceSubstreams0);
}

void startStreaming()
{
    // Open already created shared memory object.
    // Must have read and write access since we are using mutexes
    // and locking a mutex is a write operation
    shm = new boost::interprocess::managed_shared_memory(boost::interprocess::open_only,
                                                         SHM_NAME);

    cubemap = shm->find<Cubemap::Ptr>("Cubemap").first->get();

    isStoppingStreaming = false;
    env->taskScheduler().triggerEvent(addFaceSubstreamsTriggerId, NULL);
}

void stopStreaming()
{
    isStoppingStreaming = true;
    env->taskScheduler().triggerEvent(removeFaceSubstreamsTriggerId, NULL);
    stopStreamingBarrier.wait();
    delete shm;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        boost::filesystem::path exePath(argv[0]);
        std::cout << "usage: " << exePath.filename().string() << " <RTSP port>" << std::endl;
        return -1;
    }

    int rtspPort = atoi(argv[1]);

    av_log_set_level(AV_LOG_WARNING);
    avcodec_register_all();
    setupRTSP(rtspPort);
    boost::thread networkThread = boost::thread(&networkLoop);

    Process unityProcess(CUBEMAPEXTRACTIONPLUGIN_ID, false);

    while (true)
    {
        std::cout << "Waiting for Unity ..." << std::endl;
        unityProcess.waitForBirth();
        std::cout << "Connected to Unity :)" << std::endl;
        startStreaming();
        std::cout << "Streaming ..." << std::endl;
        unityProcess.join();
        std::cout << "Lost connection to Unity :(" << std::endl;
        stopStreaming();
    }

    return 0;
}