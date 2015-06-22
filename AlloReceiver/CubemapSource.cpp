#include "AlloReceiver.h"
#include "RTSPCubemapSource.hpp"

CubemapSource* CubemapSource::createFromRTSP(const char* url,
                                             int resolution,
                                             AVPixelFormat format,
                                             const char* interface)
{
    return RTSPCubemapSource::create(url, resolution, format, interface);
}

void CubemapSource::destroy(CubemapSource *cubemapSource)
{
    // to be implemented
}
