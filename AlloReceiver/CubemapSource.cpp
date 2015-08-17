#include "AlloReceiver.h"
#include "RTSPCubemapSource.hpp"

CubemapSource* CubemapSource::createFromRTSP(const char* url,
	                                         unsigned long bufferSize,
                                             int resolution,
                                             AVPixelFormat format,
                                             const char* interfaceAddress)
{
	return RTSPCubemapSource::create(url, bufferSize, resolution, format, interfaceAddress);
}

void CubemapSource::destroy(CubemapSource *cubemapSource)
{
    // to be implemented
}
