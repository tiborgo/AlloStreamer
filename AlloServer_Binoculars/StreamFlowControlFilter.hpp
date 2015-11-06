#pragma once

#include <FramedFilter.hh>

class StreamFlowControlFilter : public FramedFilter
{
public:
	static StreamFlowControlFilter* createNew(UsageEnvironment& env,
		                                      FramedSource* inputSource,
							                  unsigned long bandwidth);

protected:
	StreamFlowControlFilter(UsageEnvironment& env,
		                    FramedSource* inputSource,
							unsigned long bandwidth);

private:
	static void afterGettingFrame0(void* clientData,
		                           unsigned frameSize,
		                           unsigned numTruncatedBytes,
	                               struct timeval presentationTime,
		                           unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize,
	                       struct timeval presentationTime);
	virtual void doGetNextFrame();

	void deliverChunk();


	unsigned long bandwidth; // bandwidth is in bit per second

	const unsigned long maxChunkSize;
	unsigned long chunkIndex;
	unsigned char* buffer;
	unsigned long bufferSize;
};
