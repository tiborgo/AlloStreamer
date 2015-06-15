#include "AlloReceiver.h"
#include "RTSPCubemapSource.hpp"

CubemapSource* CubemapSource::createFromRTSP(const char* url, int resolution, AVPixelFormat format)
{
    return RTSPCubemapSource::create(url, resolution, format);
}

void CubemapSource::destroy(CubemapSource *cubemapSource)
{
    // to be implemented
}
