#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
}

#include "AlloReceiver.h"
#include "AlloShared/Cubemap.hpp"

class CubemapSource
{
public:
    //virtual StereoCubemap* getCurrentCubemap() = 0;
    virtual void setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>& nextCubemap) = 0;
    
    
    static CubemapSource* createFromRTSP(const char* url,
                                         int resolution,
                                         AVPixelFormat format,
                                         const char* interface = "0.0.0.0");
    static void destroy(CubemapSource* cubemapSource);
};
