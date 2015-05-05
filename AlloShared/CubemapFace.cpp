#include "CubemapFace.h"

AlloShared_API boost::interprocess::offset_ptr<CubemapImpl> cubemap;

boost::chrono::system_clock::time_point Frame::getPresentationTime()
{
	return presentationTime;
}