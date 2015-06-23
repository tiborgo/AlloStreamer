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
    virtual void setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>&          callback) = 0;
    virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int face, u_int8_t type)>& callback) = 0;
    virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int face, u_int8_t type)>& callback) = 0;
    
    static CubemapSource* createFromRTSP(const char* url,
                                         int resolution,
                                         AVPixelFormat format,
                                         const char* interface = "0.0.0.0");
    static void destroy(CubemapSource* cubemapSource);
};
