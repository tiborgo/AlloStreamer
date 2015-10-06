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
    
    
    class StatVal
    {
    public:
        template <typename Feature>
        static StatVal makeStatVal(boost::function<bool (TimeValueDatum)>   filter,
                                   boost::function<double (TimeValueDatum)> accessor,
                                   const Feature&                           accumulator,
                                   const std::string&                       name)
        {
            auto filterAccExtractorMaker = [filter, accessor]()
            {
                auto acc = new boost::accumulators::accumulator_set<double, boost::accumulators::features<Feature> >();
                
                auto filterAcc = [acc, filter, accessor](Stats::TimeValueDatum datum)
                {
                    if (filter(datum))
                    {
                        (*acc)(accessor(datum.value));
                    }
                };
                
                auto extractor = [acc]()
                {
                    boost::accumulators::extractor<Feature> ex;
                    return ex(*acc);
                };
                
                return std::make_pair(filterAcc, extractor);
            };
            
            return StatVal(filterAccExtractorMaker, name);
        }
        
    private:
        friend class Stats;
        
        typedef std::pair<boost::function<void (Stats::TimeValueDatum)>,  boost::function<double ()> > FilterAccExtractor;
        typedef boost::function<FilterAccExtractor ()> FilterAccExtractorMaker;
        
        StatVal(FilterAccExtractorMaker filterAccExtractorMaker,
                const std::string& name)
                :
                filterAccExtractorMaker(filterAccExtractorMaker),
                name(name) {}
        
        FilterAccExtractorMaker filterAccExtractorMaker;
        std::string name;
    };

    
    Stats();
    
    // #### STAT VALS ####
    StatVal nalusBitSum       (const std::string&                      name,
                               int                                     face,
                               NALU::Status                            status,
                               boost::chrono::microseconds             window,
                               boost::chrono::steady_clock::time_point now);
    StatVal nalusCount        (const std::string&                      name,
                               int                                     face,
                               NALU::Status                            status,
                               boost::chrono::microseconds             window,
                               boost::chrono::steady_clock::time_point now);
    StatVal cubemapsCount     (const std::string&                      name,
                               boost::chrono::microseconds             window,
                               boost::chrono::steady_clock::time_point now);
    StatVal facesCount        (const std::string&                      name,
                               int                                     face,
                               boost::chrono::microseconds             window,
                               boost::chrono::steady_clock::time_point now);

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
    
    std::map<std::string, double> query(std::initializer_list<StatVal>                         statVals,
                                        boost::function<void (std::map<std::string, double>&)> postCalculator);
    
    boost::function<bool (TimeValueDatum)> timeFilter(
        boost::chrono::microseconds window,
        boost::chrono::steady_clock::time_point now);
    
    boost::function<bool (TimeValueDatum)> typeFilter(const std::type_info& type);
	boost::function<bool (TimeValueDatum)> naluFaceFilter(int face);
    boost::function<bool (TimeValueDatum)> naluStatusFilter(NALU::Status status);
    boost::function<bool (TimeValueDatum)> andFilter(std::initializer_list<boost::function<bool (TimeValueDatum)> > filters);
    
    boost::chrono::microseconds nowSinceEpoch();
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(boost::chrono::microseconds frequency);
};

