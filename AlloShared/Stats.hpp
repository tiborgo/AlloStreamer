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
    void droppedNALU(int type, size_t size, int face);
    void addedNALU(int type, size_t size, int face);
	void sentNALU(int type, size_t size, int face);
	/*void decodedNALU(int type);
    void failedToDecodeNALU(int type);*/
    void displayedCubemapFace(int face);
    void displayedFrame();
    
    // statistical values
    double naluDropRate(boost::chrono::microseconds window,
                        boost::chrono::microseconds nowSinceEpoch);
    double facesPS(int face,
                   boost::chrono::microseconds window,
				   boost::chrono::microseconds nowSinceEpoch);
    double fps(boost::chrono::microseconds window,
		       boost::chrono::microseconds nowSinceEpoch);
    double receivedNALUsPS(int face,
		                   boost::chrono::microseconds window,
		                   boost::chrono::microseconds nowSinceEpoch);
	double processedNALUsPS(int face,
	                        boost::chrono::microseconds window,
		                    boost::chrono::microseconds nowSinceEpoch);
	double sentNALUsPS(int face,
		               boost::chrono::microseconds window,
		               boost::chrono::microseconds nowSinceEpoch);
	double receivedNALUsBitRate(boost::chrono::microseconds window,
		                        boost::chrono::microseconds nowSinceEpoch);
	double processedNALUsBitRate(boost::chrono::microseconds window,
		                         boost::chrono::microseconds nowSinceEpoch);
	double sentNALUsBitRate(boost::chrono::microseconds window,
		                    boost::chrono::microseconds nowSinceEpoch);
    
    // utility
    std::string summary(boost::chrono::microseconds window);
    void autoSummary(boost::chrono::microseconds frequency);
	void stopAutoSummary();

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
		NALU(int type, size_t size, int face);
		int type;
		size_t size;
		int face;
	};
    
	std::vector<TimeValueDatum<NALU> > droppedNALUs;
	std::vector<TimeValueDatum<NALU> > addedNALUs;
	std::vector<TimeValueDatum<NALU> > sentNALUs;
    std::vector<TimeValueDatum<int> > displayedCubemapFaces;
    std::vector<TimeValueDatum<int> > displayedFrames;
    
	template <typename Features, typename ValueType, typename AccType>
    boost::accumulators::accumulator_set<AccType, Features> filter(
       std::vector<TimeValueDatum<ValueType> >& data,
       std::initializer_list<boost::function<bool (TimeValueDatum<ValueType>)> > filters,
	   boost::function<AccType(ValueType)> accExtractor);
    
    template <typename ValueType>
    boost::function<bool (TimeValueDatum<ValueType>)> timeFilter(
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch);

	boost::function<bool(TimeValueDatum<NALU>)> faceFilter(int face);
    
    boost::chrono::microseconds nowSinceEpoch();
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(boost::chrono::microseconds frequency);
};

