#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
}

#include "AlloReceiver.h"
#include "AlloShared/CubemapFace.h"

class CubemapSource
{
public:
    virtual StereoCubemap* getCurrentCubemap() = 0;
    
    static CubemapSource* createFromRTSP(const char* url, int resolution, AVPixelFormat format);
    static destroy(CubemapSource* cubemapSource);
};