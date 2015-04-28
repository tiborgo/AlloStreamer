

#include <iostream>
#include "H264WindowSink.h"


H264WindowSink* H264WindowSink::createNew(UsageEnvironment& env,
	unsigned int bufferSize)
{
	return new H264WindowSink(env, bufferSize);
}

H264WindowSink::H264WindowSink(UsageEnvironment& env,
	unsigned int bufferSize)
	: MediaSink(env), bufferSize(bufferSize), buffer(new unsigned char[bufferSize])
{

}

void H264WindowSink::afterGettingFrame(unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime)
{
	std::cout << this << ": got frame; size: " << frameSize << std::endl;

	// Then try getting the next frame:
	continuePlaying();
}

Boolean H264WindowSink::continuePlaying()
{
	fSource->getNextFrame(buffer, bufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);

	return True;
}

void H264WindowSink::afterGettingFrame(void*clientData,
	unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime,
	unsigned /*durationInMicroseconds*/)
{
	H264WindowSink* sink = (H264WindowSink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}