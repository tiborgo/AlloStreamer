#include "AlloReceiver.h"
#include "H264CubemapSource.h"

CubemapSource* CubemapSource::createFromRTSP(const char* url, int resolution, AVPixelFormat format)
{
    return new H264CubemapSource(url, resolution, format);
}

void CubemapSource::destroy(CubemapSource *cubemapSource)
{
    // to be implemented
}
