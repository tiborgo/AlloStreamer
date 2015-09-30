#pragma once

#include <vector>
#include <boost/chrono/duration.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <initializer_list>
#include <boost/thread.hpp>
#include <boost/any.hpp>

class Stats
{
public:
    class TimeValueDatum
    {
    public:
        TimeValueDatum(const boost::any& value) : timeSinceEpoch(boost::chrono::microseconds(boost::chrono::system_clock::now().time_since_epoch().count())), value(value) {}
        const boost::chrono::microseconds timeSinceEpoch;
        const boost::any value;
    };
    
    class NALU
    {
    public:
        enum Status {DROPPED, ADDED, PROCESSED, SENT};
        
        NALU(int type, size_t size, int face, Status status) : type(type), size(size), face(face), status(status) {}
        int type;
        size_t size;
        int face;
        Status status;
    };
    
    class CubemapFace
    {
    public:
        CubemapFace(int face) : face(face) {}
        int face;
    };

    class Cubemap
    {
    };

    // events
    void store(const boost::any& datum);
    
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
    
    
    
    std::list<TimeValueDatum> storage;

	
    
	template <typename Feature>
    double filter(std::initializer_list<boost::function<bool (TimeValueDatum)> > filters,
                  const Feature& accumulator,
                  boost::function<double (TimeValueDatum)> accExtractor);
    
    boost::function<bool (TimeValueDatum)> timeFilter(
        boost::chrono::microseconds window,
        boost::chrono::microseconds nowSinceEpoch);
    
    boost::function<bool (TimeValueDatum)> typeFilter(const std::type_info& type);
    
    boost::function<bool (TimeValueDatum)> andFilter(std::initializer_list<boost::function<bool (TimeValueDatum)> > filters);

	//boost::function<bool(TimeValueDatum)> faceFilter(int face);
    
    boost::chrono::microseconds nowSinceEpoch();
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(boost::chrono::microseconds frequency);
};

