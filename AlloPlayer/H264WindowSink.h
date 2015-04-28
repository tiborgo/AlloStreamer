#pragma once
#include "MediaSink.hh"

class H264WindowSink : public MediaSink
{
public:
	static H264WindowSink* createNew(UsageEnvironment& env,
		unsigned int bufferSize);

protected:
	H264WindowSink(UsageEnvironment& env,
		unsigned int bufferSize);

	virtual void afterGettingFrame(unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime);

	virtual Boolean continuePlaying();

	static void H264WindowSink::afterGettingFrame(void*clientData,
		unsigned frameSize,
		unsigned numTruncatedBytes,
		timeval presentationTime,
		unsigned /*durationInMicroseconds*/);

private:
	unsigned int bufferSize;
	unsigned char* buffer;
};

