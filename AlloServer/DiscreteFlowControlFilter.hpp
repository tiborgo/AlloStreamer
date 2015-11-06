#pragma once

#include <FramedFilter.hh>

class DiscreteFlowControlFilter : public FramedFilter
{
public:
	static DiscreteFlowControlFilter* createNew(UsageEnvironment& env,
		                                        FramedSource* inputSource,
												unsigned long bandwidth);

protected:
	DiscreteFlowControlFilter(UsageEnvironment& env, 
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

	unsigned long bandwidth; // bandwidth is in bit per second
};
