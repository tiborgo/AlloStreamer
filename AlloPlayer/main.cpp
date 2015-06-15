
#include "DynamicCubemapBackgroundApp.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "AlloReceiver/AlloReceiver.h"

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
        ("url", boost::program_options::value<std::string>(), "url");
    
    boost::program_options::positional_options_description p;
    p.add("url", -1);
    
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    boost::program_options::notify(vm);
    
    CubemapSource* cubemapSource = CubemapSource::createFromRTSP(vm["url"].as<std::string>().c_str(), 2048, AV_PIX_FMT_RGB24);
    
    if (vm.count("no-display"))
    {
        std::cout << "network only" << std::endl;
        while (true)
        {
            cubemapSource->getCurrentCubemap();
        }
    }
    else
    {
        DynamicCubemapBackgroundApp dynamicCubemapBackgroundApp(cubemapSource);
        dynamicCubemapBackgroundApp.start();
    }
    
    CubemapSource::destroy(cubemapSource);
}
