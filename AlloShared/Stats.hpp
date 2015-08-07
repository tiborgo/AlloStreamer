#pragma once

#include <vector>
#include <boost/chrono/duration.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <initializer_list>
#include <boost/thread.hpp>

class Stats
{
public:
    // events
    void droppedNALU(int type, size_t size);
    void addedNALU(int type, size_t size);
    /*void decodedNALU(int type);
    void failedToDecodeNALU(int type);*/
    void displayedCubemapFace(int face);
    void displayedFrame();
    
    // statistical values
    double naluDropRate(boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch = boost::chrono::microseconds(0));
    double cubemapFaceFramesPS(int face,
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch = boost::chrono::microseconds(0));
    double fps(boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch = boost::chrono::microseconds(0));
    double receivedNALUsPS(boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch = boost::chrono::microseconds(0));
    double processedNALUsPS(boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch = boost::chrono::microseconds(0));
    
    // utility
    std::string summary(boost::chrono::microseconds window);
    void autoSummary(boost::chrono::microseconds frequency);

private:
    
    template <typename ValueType>
    class TimeValueDatum
    {
    public:
        TimeValueDatum(ValueType value);
        const boost::chrono::microseconds timeSinceEpoch;
        const ValueType value;
    };

	class NALU
	{
	public:
		NALU(int type, size_t size);
		int type;
		size_t size;
	};
    
	std::vector<TimeValueDatum<NALU> > droppedNALUs;
	std::vector<TimeValueDatum<NALU> > addedNALUs;
    std::vector<TimeValueDatum<int> > displayedCubemapFaces;
    std::vector<TimeValueDatum<int> > displayedFrames;
    
    template <typename ValueType>
    bool isInTime(Stats::TimeValueDatum<ValueType> datum,
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch);
    
    template <typename Features, typename ValueType>
    boost::accumulators::accumulator_set<Stats::TimeValueDatum<ValueType>, Features>filterTime(
        std::vector<TimeValueDatum<ValueType> >& data,
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch);
    
    template <typename Features, typename ValueType>
    boost::accumulators::accumulator_set<Stats::TimeValueDatum<ValueType>, Features> filter(
       std::vector<TimeValueDatum<ValueType> >& data,
       std::initializer_list<boost::function<bool (TimeValueDatum<ValueType>)> > filters);
    
    template <typename ValueType>
    boost::function<bool (TimeValueDatum<ValueType>)> timeFilter(
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch);
    
    template <typename Features>
    boost::accumulators::accumulator_set<TimeValueDatum<int>, Features> filterTimeFace(
        std::vector<TimeValueDatum<int> >& data,
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch,
        int face);
    
    boost::chrono::microseconds nowSinceEpoch();
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
    void autoSummaryLoop(boost::chrono::microseconds frequency);
};

