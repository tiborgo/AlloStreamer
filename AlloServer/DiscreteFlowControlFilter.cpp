#include <boost/chrono.hpp>
#include <boost/predef.h>

#include "DiscreteFlowControlFilter.hpp"

DiscreteFlowControlFilter* DiscreteFlowControlFilter::createNew(UsageEnvironment& env,
	                                                            FramedSource* inputSource,
																unsigned long bandwidth)
{
	return new DiscreteFlowControlFilter(env, inputSource, bandwidth);
}

DiscreteFlowControlFilter::DiscreteFlowControlFilter(UsageEnvironment& env,
	                                                FramedSource* inputSource,
													unsigned long bandwidth)
	:
	FramedFilter(env, inputSource), bandwidth(bandwidth)
{
}

void DiscreteFlowControlFilter::doGetNextFrame()
{
	// Read directly from our input source into our client's buffer:
	fFrameSize = 0;
	fInputSource->getNextFrame(fTo,
		                       fMaxSize,
		                       afterGettingFrame0,
							   this,
		                       FramedSource::handleClosure,
							   this);
}

void DiscreteFlowControlFilter::afterGettingFrame0(void* clientData,
	                                               unsigned frameSize,
                                                   unsigned numTruncatedBytes,
                                                   struct timeval presentationTime,
	                                               unsigned durationInMicroseconds)
{
	DiscreteFlowControlFilter* framer = (DiscreteFlowControlFilter*)clientData;
	framer->afterGettingFrame(frameSize, presentationTime);
}

void DiscreteFlowControlFilter::afterGettingFrame(unsigned frameSize,
                                                  struct timeval presentationTime)
{
	fFrameSize = frameSize;
	fPresentationTime = presentationTime;

#if BOOST_OS_WINDOWS
	auto durationPerKBit = boost::chrono::nanoseconds(boost::ratio_multiply<boost::giga, boost::kilo>::num / bandwidth);

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);
#endif

	// Activity to be timed
	afterGetting(this);

#if BOOST_OS_WINDOWS
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//

	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	auto isDuration = boost::chrono::microseconds(ElapsedMicroseconds.QuadPart);
	auto shouldDuration = durationPerKBit * fFrameSize * 8 / 1000;
	auto coolDownDuration = shouldDuration - isDuration;

	// Let network cool down
	QueryPerformanceCounter(&StartingTime);
	boost::chrono::microseconds elapsedCoolDownDuration;
	do
	{
		QueryPerformanceCounter(&EndingTime);
		ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicroseconds.QuadPart *= 1000000;
		ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
		elapsedCoolDownDuration = boost::chrono::microseconds(ElapsedMicroseconds.QuadPart);
	}
	while (elapsedCoolDownDuration < coolDownDuration);
#endif
}