
#include "Renderer.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/ref.hpp>

#include "AlloShared/StatsUtils.hpp"
#include "AlloShared/to_human_readable_byte_count.hpp"
#include "AlloReceiver/RTSPCubemapSourceClient.hpp"
#include "AlloReceiver/AlloReceiver.h"
#include "AlloReceiver/Stats.hpp"
#include "AlloReceiver/H264CubemapSource.h"

const unsigned int DEFAULT_SINK_BUFFER_SIZE = 2000000000;

static Stats stats;
static bool noDisplay;
static boost::barrier barrier(2);
static CubemapSource* cubemapSource;
static RTSPCubemapSourceClient* rtspClient;

StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
    for (int i = 0; i < cubemap->getEye(0)->getFacesCount(); i++)
    {
        stats.store(StatsUtils::CubemapFace(i, StatsUtils::CubemapFace::DISPLAYED));
    }
    stats.store(StatsUtils::Cubemap());
    return cubemap;
}

void onReceivedNALU(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::NALU(type, size, face, StatsUtils::NALU::RECEIVED));
}

void onReceivedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::RECEIVED));
}

void onDecodedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::DECODED));
}

void onColorConvertedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::COLOR_CONVERTED));
}

void onAddedFrameToCubemap(CubemapSource* source, int face)
{
    //stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::ADDED));
}

void setOnScheduledFrameInCubemap(CubemapSource* source, int face)
{
    stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::SCHEDULED));
}

void onDisplayedCubemapFace(Renderer* renderer, int face)
{
    stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::DISPLAYED));
}

void onDisplayedFrame(Renderer* renderer)
{
    stats.store(StatsUtils::Cubemap());
}

void onDidConnect(RTSPCubemapSourceClient* client, CubemapSource* cubemapSource)
{
    H264CubemapSource* h264CubemapSource = dynamic_cast<H264CubemapSource*>(cubemapSource);
    if (h264CubemapSource)
    {
        h264CubemapSource->setOnReceivedNALU           (boost::bind(&onReceivedNALU,               _1, _2, _3, _4));
        h264CubemapSource->setOnReceivedFrame          (boost::bind(&onReceivedFrame,              _1, _2, _3, _4));
        h264CubemapSource->setOnDecodedFrame           (boost::bind(&onDecodedFrame,               _1, _2, _3, _4));
        h264CubemapSource->setOnColorConvertedFrame    (boost::bind(&onColorConvertedFrame,        _1, _2, _3, _4));
        h264CubemapSource->setOnAddedFrameToCubemap    (boost::bind(&onAddedFrameToCubemap,        _1, _2));
        h264CubemapSource->setOnScheduledFrameInCubemap(boost::bind(&setOnScheduledFrameInCubemap, _1, _2));
    }
    
    
    /*stats.autoSummary(boost::chrono::seconds(10),
                      AlloReceiver::statValsMaker,
                      AlloReceiver::postProcessorMaker,
                      AlloReceiver::formatStringMaker);*/
    
    ::cubemapSource = cubemapSource;
    
    barrier.wait();
}

void inLoop()
{
    boost::program_options::options_description desc("hadkhaksjhakshkjh");
    desc.add_options()
    ("no-display", "")
    ("url", boost::program_options::value<std::string>(), "url")
    ("interface", boost::program_options::value<std::string>(), "interface")
    ("buffer-size", boost::program_options::value<unsigned long>(), "buffer-size")
    ("match-stereo-pairs", "")
    ("help", "")
    ("stats", "");
    
    //boost::program_options::positional_options_description p;
    //p.add("url", -1);
    
    
    /*boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
                                  options(desc).positional(p).run(), vm);
    boost::program_options::notify(vm);*/
    
    boost::chrono::steady_clock::time_point lastStatsTime = boost::chrono::steady_clock::now();
    
    while (true)
    {
        std::string input;
        std::cout << ">";
        std::getline(std::cin, input);
        
        input = "--" + input;
        
        boost::program_options::variables_map vm;
        
        //std::istringstream ifs(input);
        const char* inputStr[] = {"bla", input.c_str()};
        boost::program_options::store(boost::program_options::parse_command_line(2, inputStr, desc), vm);
        notify(vm);
        
        if (vm.count("help"))
        {
            std::cout << vm["help"].as<std::string>() << " yes ";
            std::cout << desc << std::endl;
        }
        
        if (vm.count("stats"))
        {
            boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
            std::cout << stats.summary(boost::chrono::duration_cast<boost::chrono::microseconds>(now - lastStatsTime),
                                       AlloReceiver::statValsMaker,
                                       AlloReceiver::postProcessorMaker,
                                       AlloReceiver::formatStringMaker);
            lastStatsTime = now;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        boost::filesystem::path exePath(argv[0]);
        std::cout << "usage: " << exePath.filename().string() << " <RTSP url of stream>" << std::endl;
        std::cout << "\texample: " << exePath.filename().string() << " rtsp://192.168.1.185:8554/h264ESVideoTest" << std::endl;
        return -1;
    }
    
    boost::program_options::options_description desc("");
    desc.add_options()
        ("no-display", "")
        ("url", boost::program_options::value<std::string>(), "url")
        ("interface", boost::program_options::value<std::string>(), "interface")
        ("buffer-size", boost::program_options::value<unsigned long>(), "buffer-size")
        ("match-stereo-pairs", "");
    
    boost::program_options::positional_options_description p;
    p.add("url", -1);
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    boost::program_options::notify(vm);
    
    const char* interface;
    if (vm.count("interface"))
    {
        interface = vm["interface"].as<std::string>().c_str();
    }
    else
    {
        interface = "0.0.0.0";
    }
    
    unsigned long bufferSize;
    if (vm.count("buffer-size"))
    {
        bufferSize = vm["buffer-size"].as<unsigned long>();
    }
    else
    {
        bufferSize = DEFAULT_SINK_BUFFER_SIZE;
    }
    std::cout << "Buffer size " << to_human_readable_byte_count(bufferSize, false, false) << std::endl;

    if (vm.count("no-display"))
    {
        noDisplay = true;
    }
    else
    {
        noDisplay = false;
    }
    
    bool matchStereoPairs;
    if (vm.count("match-stereo-pairs"))
    {
        matchStereoPairs = true;
    }
    else
    {
        matchStereoPairs = false;
    }
    
    rtspClient = RTSPCubemapSourceClient::create(vm["url"].as<std::string>().c_str(),
                                                 bufferSize,
                                                 AV_PIX_FMT_YUV420P,
                                                 matchStereoPairs,
                                                 interface);
    std::function<void (RTSPCubemapSourceClient*, CubemapSource*)> callback(boost::bind(&onDidConnect, _1, _2));
    rtspClient->setOnDidConnect(callback);
    rtspClient->connect();
    
    barrier.wait();
    
    boost::thread inThread(boost::bind(&inLoop));
    
    if (noDisplay)
    {
        std::cout << "network only" << std::endl;
        std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)> callback = boost::bind(&onNextCubemap, _1, _2);
        cubemapSource->setOnNextCubemap(callback);
        
        while(true)
        {
            boost::this_thread::yield();
        }
    }
    else
    {
        Renderer renderer(cubemapSource);
        std::function<void (Renderer*, int)> onDisplayedCubemapFaceCallback = boost::bind(&onDisplayedCubemapFace, _1, _2);
        renderer.setOnDisplayedCubemapFace(onDisplayedCubemapFaceCallback);
        std::function<void (Renderer*)> onDisplayedFrameCallback = boost::bind(&onDisplayedFrame, _1);
        renderer.setOnDisplayedFrame(onDisplayedFrameCallback);
        renderer.start();
    }

    CubemapSource::destroy(cubemapSource);
}
