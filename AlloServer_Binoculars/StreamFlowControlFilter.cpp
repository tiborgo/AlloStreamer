#include <boost/chrono.hpp>
#include <boost/predef.h>
#include <algorithm>

#include "StreamFlowControlFilter.hpp"

StreamFlowControlFilter* StreamFlowControlFilter::createNew(UsageEnvironment& env,
	                                                            FramedSource* inputSource,
																unsigned long bandwidth)
{
	return new StreamFlowControlFilter(env, inputSource, bandwidth);
}

StreamFlowControlFilter::StreamFlowControlFilter(UsageEnvironment& env,
	FramedSource* inputSource,
	unsigned long bandwidth)
	:
	FramedFilter(env, inputSource),
	bandwidth(bandwidth),
	buffer(new unsigned char[fMaxSize]),
	bufferSize(0),
	maxChunkSize(10000),
	chunkIndex(0)
{
}

void StreamFlowControlFilter::doGetNextFrame()
{
	if (chunkIndex * maxChunkSize < bufferSize)
	{
		deliverChunk();
	}
	else
	{
		// Read directly from our input source into our client's buffer:
		fFrameSize = 0;
		fInputSource->getNextFrame(buffer,
			                       fMaxSize,
			                       afterGettingFrame0,
			                       this,
			                       FramedSource::handleClosure,
			                       this);
	}
}

void StreamFlowControlFilter::afterGettingFrame0(void* clientData,
	                                               unsigned frameSize,
                                                   unsigned numTruncatedBytes,
                                                   struct timeval presentationTime,
	                                               unsigned durationInMicroseconds)
{
	StreamFlowControlFilter* framer = (StreamFlowControlFilter*)clientData;
	framer->afterGettingFrame(frameSize, presentationTime);
}

void StreamFlowControlFilter::afterGettingFrame(unsigned frameSize,
                                                struct timeval presentationTime)
{
	bufferSize = frameSize;
	fPresentationTime = presentationTime;
	chunkIndex = 0;
	deliverChunk();
}

void StreamFlowControlFilter::deliverChunk()
{
	// Fill buffer with next chunk
	fFrameSize = (std::min)(bufferSize - chunkIndex * maxChunkSize, maxChunkSize);
	memcpy(fTo, buffer + (chunkIndex * maxChunkSize), fFrameSize);
	chunkIndex++;

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
	} while (elapsedCoolDownDuration < coolDownDuration);
#endif
}