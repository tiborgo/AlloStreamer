#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
}

#include "Stats.h"
#include "AlloShared/CubemapFace.h"

extern Stats stats;

class CubemapSource
{
public:
    virtual StereoCubemap* tryGetNextCubemap(int desiredResolution, AVPixelFormat desiredFormat) = 0;
    virtual StereoCubemap* tryGetNextCubemap() = 0;
    
    static CubemapSource* createFromRTSP(const char* url);
    //static destroy(CubemapSource* cubemapSource);
};