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
    //Cubemap* tryGetNextCubemap();
    virtual AVFrame* tryGetNextFace(int face) = 0;
    virtual int getFacesCount() = 0;
    
    static CubemapSource* createFromRTSP(const char* url);
    //static destroy(CubemapSource* cubemapSource);
};