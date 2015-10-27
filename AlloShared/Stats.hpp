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
    
    class StatVal
    {
    public:
        template <typename Feature>
        static StatVal makeStatVal(boost::function<bool (const TimeValueDatum&)>   filter,
                                   boost::function<double (const TimeValueDatum&)> accessor,
                                   const Feature&                                  accumulator,
                                   const std::string&                              name)
        {
            auto filterAccExtractorMaker = [filter, accessor]()
            {
                auto acc = new boost::accumulators::accumulator_set<double, boost::accumulators::features<Feature> >();
                
                auto filterAcc = [acc, filter, accessor](const Stats::TimeValueDatum& datum)
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
                
				return FilterAccExtractor(filterAcc, extractor);
            };
            
            return StatVal(filterAccExtractorMaker, name);
        }
        
    private:
        friend class Stats;
        
        typedef std::pair<boost::function<void (const Stats::TimeValueDatum&)>, boost::function<double ()> > FilterAccExtractor;
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

    // events
    void store(const boost::any& datum);
    
    // utility

	typedef boost::function<void(std::map<std::string, double>&)> PostProcessor;
	typedef boost::function<std::list<Stats::StatVal>(boost::chrono::microseconds,
		                                              boost::chrono::steady_clock::time_point)> StatValsMaker;
	typedef boost::function<PostProcessor(boost::chrono::microseconds,
		                                  boost::chrono::steady_clock::time_point)> PostProcessorMaker;

	std::string summary(boost::chrono::microseconds window,
		                StatValsMaker               statValsMaker,
		                PostProcessorMaker          postProcessorMaker,
		                const std::string&          format);
    void autoSummary(boost::chrono::microseconds frequency,
					 StatValsMaker               statValsMaker,
					 PostProcessorMaker          postProcessorMaker,
                     const std::string&          format);
	void stopAutoSummary();

private:
    std::list<TimeValueDatum>* activeStorage;
    std::list<TimeValueDatum>* processingStorage;
    
    std::list<TimeValueDatum> storage1;
    std::list<TimeValueDatum> storage2;
    
    std::map<std::string, double> query(std::initializer_list<StatVal>                         statVals,
                                        boost::function<void (std::map<std::string, double>&)> postCalculator);
	std::map<std::string, double> query(std::list<StatVal>                                    statVals,
									    boost::function<void(std::map<std::string, double>&)> postCalculator);
  
    
    std::string formatDuration(boost::chrono::microseconds duration);
    
    boost::mutex mutex;
    boost::thread autoSummaryThread;
	bool stopAutoSummary_;
    void autoSummaryLoop(boost::chrono::microseconds frequency,
						 StatValsMaker               statValsMaker,
						 PostProcessorMaker          postProcessorMaker,
						 const std::string&          format);
};

