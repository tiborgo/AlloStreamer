
#include "Renderer.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/ref.hpp>

#include "AlloShared/Stats.hpp"
#include "AlloShared/to_human_readable_byte_count.hpp"
#include "AlloReceiver/AlloReceiver.h"

const unsigned int DEFAULT_SINK_BUFFER_SIZE = 2000000000;

Stats stats;

StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
    for (int i = 0; i < cubemap->getEye(0)->getFacesCount(); i++)
    {
        stats.displayedCubemapFace(i);
    }
    stats.displayedFrame();
    StereoCubemap::destroy(cubemap);
    return nullptr;
}

void onDroppedNALU(CubemapSource* source, int face, u_int8_t type, size_t size)
{
    stats.droppedNALU(type, size, face);
}

void onAddedNALU(CubemapSource* source, int face, u_int8_t type, size_t size)
{
    stats.addedNALU(type, size, face);
}

void onDisplayedCubemapFace(Renderer* renderer, int face)
{
    stats.displayedCubemapFace(face);
}

void onDisplayedFrame(Renderer* renderer)
{
    stats.displayedFrame();
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
        ("buffer-size", boost::program_options::value<unsigned long>(), "buffer-size");
    
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

    CubemapSource* cubemapSource = CubemapSource::createFromRTSP(vm["url"].as<std::string>().c_str(), bufferSize, 2048, AV_PIX_FMT_RGBA, interface);
    
    std::function<void (CubemapSource*, int, u_int8_t, size_t)> callback = boost::bind(&onDroppedNALU, _1, _2, _3, _4);
    cubemapSource->setOnDroppedNALU(callback);
    callback = boost::bind(&onAddedNALU, _1, _2, _3, _4);
    cubemapSource->setOnAddedNALU(callback);
    
    stats.autoSummary(boost::chrono::seconds(10));
    
    if (vm.count("no-display"))
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
