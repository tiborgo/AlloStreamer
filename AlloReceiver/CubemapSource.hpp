#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
}

#include "AlloReceiver.h"
#include "AlloShared/Cubemap.hpp"

class ALLORECEIVER_API CubemapSource
{
public:
    virtual void setOnNextCubemap(std::function<void (CubemapSource*, StereoCubemap*)>& callback) = 0;
    virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback) = 0;
    virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback) = 0;
    
    static CubemapSource* createFromRTSP(const char* url,
		                                 unsigned long bufferSize,
                                         int resolution,
                                         AVPixelFormat format,
                                         const char* interfaceAddress = "0.0.0.0");
    static void destroy(CubemapSource* cubemapSource);
};
