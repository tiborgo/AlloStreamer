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
    // PARAMETERS
    class CubemapFace
    {
    public:
        CubemapFace(int face) : face(face) {}
        int face;
    };
    
    class NALU
    {
    public:
        enum Status {DROPPED, ADDED, PROCESSED};
        
        NALU(int type, size_t size, int face, Status status) : type(type), size(size), face(face), status(status) {}
        int type;
        size_t size;
        int face;
        Status status;
    };
    
    class Cubemap
    {
    };
    
    // QUERYING
    class TimeValueDatum
    {
    public:
        TimeValueDatum(const boost::any& value) : timeSinceEpoch(boost::chrono::duration_cast<boost::chrono::microseconds>(boost::chrono::system_clock::now().time_since_epoch())),
                                                  value(value) {}
        const boost::chrono::microseconds timeSinceEpoch;
        const boost::any value;
    };
    
    std::vector<double> query(const std::vector<boost::function<bool   (TimeValueDatum&)> >& filters,
                              const std::vector<boost::function<double (TimeValueDatum&)> >& accumulators);
    std::vector<double> query(std::initializer_list<boost::function<bool   (TimeValueDatum&)> > filters,
                              std::initializer_list<boost::function<double (TimeValueDatum&)> > accumulators);
    
    
    // EVENTS
    void store(const boost::any& datum);
    
    // FILTERS
    boost::function<bool (TimeValueDatum&)> timeFilter(boost::chrono::microseconds window,
                                                       boost::chrono::microseconds nowSinceEpoch);
    
    // ACCUMULATORS
    boost::function<double (TimeValueDatum&)> averageAcc();
    boost::function<double (TimeValueDatum&)> minAcc();
    boost::function<double (TimeValueDatum&)> maxAcc();
    
    // UTILITY
    std::string summary(std::initializer_list</*std::initializer_list<*/boost::function<bool   (TimeValueDatum&)> /*>*/ >     filters,
                        std::initializer_list</*std::initializer_list<*/boost::function<double (TimeValueDatum&)> /*>*/ >     accumulators,
                        const std::string&                                                                                   format,
                        boost::chrono::microseconds                                                                          window,
                        boost::chrono::microseconds                                                                          nowSinceEpoch);
    std::string summary(std::vector</*std::initializer_list<*/boost::function<bool   (TimeValueDatum&)> /*>*/ >     filters,
                        std::vector</*std::initializer_list<*/boost::function<double (TimeValueDatum&)> /*>*/ >     accumulators,
                        const std::string&                                                                                   format,
                        boost::chrono::microseconds                                                                          window,
                        boost::chrono::microseconds                                                                          nowSinceEpoch);
    
    void startAutoSummary(std::initializer_list</*std::initializer_list<*/boost::function<bool   (TimeValueDatum&)> /*>*/ >     filters,
                          std::initializer_list</*std::initializer_list<*/boost::function<double (TimeValueDatum&)> /*>*/ >     accumulators,
                          const std::string&                                                                                   format,
                          boost::chrono::microseconds                                                                          interval);
    void startAutoSummary(std::vector</*std::initializer_list<*/boost::function<bool   (TimeValueDatum&)> /*>*/ >     filters,
                          std::vector</*std::initializer_list<*/boost::function<double (TimeValueDatum&)> /*>*/ >     accumulators,
                          const std::string&                                                                                   format,
                          boost::chrono::microseconds                                                                          interval);
	void stopAutoSummary();
    
private:
    std::list<TimeValueDatum> storage;
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(std::vector</*std::initializer_list<*/boost::function<bool   (TimeValueDatum&)> /*>*/ >    filters,
                         std::vector</*std::initializer_list<*/boost::function<double (TimeValueDatum&)> /*>*/ >     accumulators,
                         std::string                                                                                   format,
                         boost::chrono::microseconds                                                                          interval);
    std::string formatDuration(boost::chrono::microseconds duration);
};

