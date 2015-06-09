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
#include "AlloShared/CubemapFace.h"
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
static EventTriggerId addFaceSubstreamTriggerId;
static concurrent_queue<CubemapFace*> faceBuffer;
static struct in_addr destinationAddress;

static void afterPlaying(void* clientData)
{
    FaceStreamState* state = (FaceStreamState*) clientData;

    *env << "stopped streaming face " << state->face->index << "\n";

    state->sink->stopPlaying();
    Medium::close(state->source);
    // Note that this also closes the input file that this source read from.

    delete state;
}

void addFaceSubstream0(void*)
{

    CubemapFace* face;

    while (faceBuffer.try_pop(face))
    {
        FaceStreamState* state = new FaceStreamState;
        state->face = face;

        Port rtpPort(RTP_PORT_NUM + face->index);
        Groupsock* rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPort, TTL);
        rtpGroupsock->multicastSendOnly(); // we're a SSM source

        // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
        // OutPacketBuffer::maxSize = 100000;
        state->sink = H264VideoRTPSink::createNew(*env, rtpGroupsock, 96);

        ServerMediaSubsession* subsession = PassiveServerMediaSubsession::createNew(*state->sink);

        sms->addSubsession(subsession);

        state->source =  H264VideoStreamDiscreteFramer::createNew(*env, CubemapFaceSource::createNew(*env, face));
        state->sink->startPlaying(*state->source, afterPlaying, state);

        std::cout << "added face " << face->index << std::endl;
    }
}

void addFaceSubstream()
{
    std::list<int> addedFaces;

    while (true)
    {
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(cubemap->mutex);
        //
        for (int i = 0; i < cubemap->count(); i++)
        {
            if (std::find(addedFaces.begin(), addedFaces.end(), cubemap->getFace(i)->index) == addedFaces.end())
            {
                faceBuffer.push(cubemap->getFace(i).get());
                addedFaces.push_back(cubemap->getFace(i)->index);
            }
        }
        if (!faceBuffer.empty())
        {
            env->taskScheduler().triggerEvent(addFaceSubstreamTriggerId, NULL);
        }
        cubemap->newFaceCondition.wait(lock);
    }
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
                           char const* streamName)
{
    char* url = rtspServer->rtspURL(sms);
    UsageEnvironment& env = rtspServer->envir();

    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

void serverLoop(int port)
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
    RTSPServer* rtspServer = RTSPServer::createNew(*env, port, NULL);

    if (rtspServer == NULL)
    {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    char const* descriptionString
        = "Session streamed by \"testOnDemandRTSPServer\"";

    // Set up each of the possible streams that can be served by the
    // RTSP server.  Each such stream is implemented using a
    // "ServerMediaSession" object, plus one or more
    // "ServerMediaSubsession" objects for each audio/video substream.


    OutPacketBuffer::maxSize = 4000000;


    char const* streamName = "h264ESVideoTest";

    sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                        descriptionString, True);


    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName);


    addFaceSubstreamTriggerId = env->taskScheduler().createEventTrigger(&addFaceSubstream0);

    boost::thread thread2(&addFaceSubstream);

    env->taskScheduler().doEventLoop(); // does not return
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        boost::filesystem::path exePath(argv[0]);
        std::cout << "usage: " << exePath.filename().string() << " <RTSP port>" << std::endl;
        return -1;
    }
    
    Process thisProcess(ALLOSERVER_ID, true);
    Process unityProcess(CUBEMAPEXTRACTIONPLUGIN_ID, false);
    if (unityProcess.isAlive())
    {
        std::cout << "Unity is already running :)" << std::endl;
    }
    else
    {
        std::cout << "Unity not started :(" << std::endl;
        std::cout << "wait for Unity to be born..." << std::endl;
        unityProcess.waitForBirth();
        std::cout << "Unity born :)" << std::endl;
    }

    avcodec_register_all();

    // Open already created shared memory object.
    // Must have read and write access since we are using mutexes
    // and locking a mutex is a write operation
    boost::interprocess::managed_shared_memory shm =
        boost::interprocess::managed_shared_memory(boost::interprocess::open_only,
                                                   SHM_NAME);

    cubemap = shm.find<CubemapImpl>("Cubemap").first;

    int port = atoi(argv[1]);
    serverLoop(port); // does not return

    return 0;
}