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
        TimeValueDatum(const boost::any& value) : time(boost::chrono::steady_clock::now()), value(value) {}
		const boost::chrono::steady_clock::time_point time;
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
    
    Stats();

    // events
    void store(const boost::any& datum);
    
    // statistical values
    double naluDropRate(boost::chrono::microseconds window,
		boost::chrono::steady_clock::time_point now);
    double facesPS(int face,
                   boost::chrono::microseconds window,
				   boost::chrono::steady_clock::time_point now);
    double fps(boost::chrono::microseconds window,
		boost::chrono::steady_clock::time_point now);
    double receivedNALUsPS(int face,
		                   boost::chrono::microseconds window,
						   boost::chrono::steady_clock::time_point now);
	double processedNALUsPS(int face,
	                        boost::chrono::microseconds window,
							boost::chrono::steady_clock::time_point now);
	double sentNALUsPS(int face,
		               boost::chrono::microseconds window,
					   boost::chrono::steady_clock::time_point now);
	double receivedNALUsBitRate(boost::chrono::microseconds window,
		boost::chrono::steady_clock::time_point now);
	double processedNALUsBitRate(boost::chrono::microseconds window,
		boost::chrono::steady_clock::time_point now);
	double sentNALUsBitRate(boost::chrono::microseconds window,
		boost::chrono::steady_clock::time_point now);
    
    // utility
    std::string summary(boost::chrono::microseconds window);
    void autoSummary(boost::chrono::microseconds frequency);
	void stopAutoSummary();

private:
    std::list<TimeValueDatum>* activeStorage;
    std::list<TimeValueDatum>* processingStorage;
    
    std::list<TimeValueDatum> storage1;
    std::list<TimeValueDatum> storage2;
    
	template <typename... Features>
    std::vector<double> query(std::initializer_list<boost::function<bool (TimeValueDatum)> > filters,
                              boost::function<double (TimeValueDatum)> accExtractor,
                              const Features& ... accumulators);
    
    boost::function<bool (TimeValueDatum)> timeFilter(
        boost::chrono::microseconds window,
        boost::chrono::steady_clock::time_point now);
    
    boost::function<bool (TimeValueDatum)> typeFilter(const std::type_info& type);
	boost::function<bool (TimeValueDatum)> naluFaceFilter(int face);
    
    boost::function<bool (TimeValueDatum)> andFilter(std::initializer_list<boost::function<bool (TimeValueDatum)> > filters);
    
    boost::chrono::microseconds nowSinceEpoch();
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(boost::chrono::microseconds frequency);
};

