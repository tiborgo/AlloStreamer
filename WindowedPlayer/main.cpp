

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/ref.hpp>

#include "AlloReceiver/RTSPCubemapSourceClient.hpp"
#include "AlloShared/StatsUtils.hpp"
#include "AlloReceiver/AlloReceiver.h"
#include "AlloShared/to_human_readable_byte_count.hpp"
#include "AlloReceiver/Stats.hpp"
#include "AlloReceiver/H264CubemapSource.h"

#include "Renderer.hpp"

const unsigned int DEFAULT_SINK_BUFFER_SIZE = 200000000;

static Stats stats;
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
    stats.store(StatsUtils::NALU(type, size, face, StatsUtils::NALU::RECEIVED));
}

void onReceivedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::RECEIVED));
}

void onDecodedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::DECODED));
}

void onColorConvertedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::COLOR_CONVERTED));
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
    if (typeid(cubemapSource) == typeid(H264CubemapSource))
    {
        H264CubemapSource* h264CubemapSource = (H264CubemapSource*)cubemapSource;
        
        h264CubemapSource->setOnReceivedNALU       (boost::bind(&onReceivedNALU,        _1, _2, _3, _4));
        h264CubemapSource->setOnReceivedFrame      (boost::bind(&onReceivedFrame,       _1, _2, _3, _4));
        h264CubemapSource->setOnDecodedFrame       (boost::bind(&onDecodedFrame,        _1, _2, _3, _4));
        h264CubemapSource->setOnColorConvertedFrame(boost::bind(&onColorConvertedFrame, _1, _2, _3, _4));
    }
    
    stats.autoSummary(boost::chrono::seconds(10),
					  AlloReceiver::statValsMaker,
					  AlloReceiver::postProcessorMaker,
					  AlloReceiver::formatStringMaker);
    
    ::cubemapSource = cubemapSource;
    
    barrier.wait();
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        boost::filesystem::path exePath(argv[0]);
        std::cout << "usage: " << exePath.filename().string() << " <RTSP url of stream>" << std::endl;
        return -1;
    }
    
    boost::program_options::options_description desc("");
	desc.add_options()
		("no-display", "")
		("url", boost::program_options::value<std::string>(), "url")
		("interface", boost::program_options::value<std::string>(), "interface")
		("buffer-size", boost::program_options::value<unsigned long>(), "buffer-size");
    
    boost::program_options::positional_options_description p;
    p.add("url", -1);
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    boost::program_options::notify(vm);
    
    const char* interfaceAddress;
    if (vm.count("interface"))
    {
		interfaceAddress = vm["interface"].as<std::string>().c_str();
    }
    else
    {
		interfaceAddress = "0.0.0.0";
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

	rtspClient = RTSPCubemapSourceClient::create(vm["url"].as<std::string>().c_str(), bufferSize, AV_PIX_FMT_RGBA, false, interfaceAddress);
    std::function<void (RTSPCubemapSourceClient*, CubemapSource*)> callback(boost::bind(&onDidConnect, _1, _2));
    rtspClient->setOnDidConnect(callback);
    rtspClient->connect();
    
    barrier.wait();
    
    Renderer renderer(cubemapSource);
	renderer.setOnDisplayedCubemapFace(boost::bind(&onDisplayedCubemapFace, _1, _2));
	renderer.setOnDisplayedFrame(boost::bind(&onDisplayedFrame, _1));
    renderer.start(); // Returns when window is closed
    
    CubemapSource::destroy(cubemapSource);
}
