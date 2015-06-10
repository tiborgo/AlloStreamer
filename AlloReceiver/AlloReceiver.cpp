#include "AlloReceiver.h"
#include "H264CubemapSource.h"

CubemapSource* CubemapSource::createFromRTSP(const char* url)
{
    return new H264CubemapSource(url);
}
