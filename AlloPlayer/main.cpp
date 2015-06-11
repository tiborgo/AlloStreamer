
#include "DynamicCubemapBackgroundApp.hpp"

#include <boost/filesystem.hpp>

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
    
    CubemapSource* cubemapSource = CubemapSource::createFromRTSP(argv[1], 2048, AV_PIX_FMT_RGB24);
    
    DynamicCubemapBackgroundApp dynamicCubemapBackgroundApp(cubemapSource);
    dynamicCubemapBackgroundApp.start();
}
