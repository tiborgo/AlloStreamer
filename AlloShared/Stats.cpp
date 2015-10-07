#include <boost/chrono/system_clocks.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread/thread.hpp>

#include "format.hpp"
#include "to_human_readable_byte_count.hpp"
#include "Stats.hpp"
#include "StatsUtils.hpp"

Stats::Stats()
    :
    activeStorage(&storage1),
    processingStorage(&storage2)
{
    
}

std::map<std::string, double> Stats::query(std::initializer_list<StatVal>                         statVals,
                                           boost::function<void (std::map<std::string, double>&)> postCalculator)
{
	std::list<StatVal> statValList(statVals.begin(), statVals.end());
	return query(statValList,
		         postCalculator);
}

std::map<std::string, double> Stats::query(std::list<StatVal>                         statVals,
	                                       boost::function<void(std::map<std::string, double>&)> postCalculator)
{
	// Create list of makers so that they are active for the time of stats processing
	std::list<StatVal::FilterAccExtractorMaker> faeMakers;
	for (StatVal statVal : statVals)
	{
		faeMakers.push_back(statVal.filterAccExtractorMaker);
	}

	// Make accumulators etc.
	std::list<StatVal::FilterAccExtractor> faes;
	for (auto faeMaker : faeMakers)
	{
		faes.push_back(faeMaker());
	}

	// Process stats
	for (auto datum : *processingStorage)
	{
		for (auto fae : faes)
		{
			fae.first(datum);
		}
	}

	// Get stat values
	std::map<std::string, double> results;
	auto faesIt = faes.begin();
	auto statValsIt = statVals.begin();
	for (; faesIt != faes.end() && statValsIt != statVals.end(); ++faesIt, ++statValsIt)
	{
		results[statValsIt->name] = faesIt->second();
	}

	// Make post calculations
	postCalculator(results);

	return results;
}

std::string Stats::formatDuration(boost::chrono::microseconds duration)
{
    std::stringstream result;
	boost::chrono::microseconds remainder(duration);

	boost::chrono::hours h = boost::chrono::duration_cast<boost::chrono::hours>(remainder);

    if (h.count() > 0)
    {
        result << h.count() << "h ";
        remainder -= h;
    }

	boost::chrono::minutes m = boost::chrono::duration_cast<boost::chrono::minutes>(remainder);
    if (m.count() > 0)
    {
        result << m.count() << "m ";
        remainder -= m;
    }

	boost::chrono::seconds s = boost::chrono::duration_cast<boost::chrono::seconds>(remainder);
    if (s.count() > 0)
    {
        result << s.count() << "s ";
        remainder -= s;
    }

	boost::chrono::milliseconds ms = boost::chrono::duration_cast<boost::chrono::milliseconds>(remainder);
    if (ms.count() > 0)
    {
        result << ms.count() << "ms ";
        remainder -= ms;
    }

	boost::chrono::microseconds us = boost::chrono::duration_cast<boost::chrono::microseconds>(remainder);
    if (us.count() > 0)
    {
        result << us.count() << "µs ";
        remainder -= us;
    }

    std::string format = result.str();
    return format.substr(0, format.size() - 1);
}

// ###### EVENTS ######

void Stats::store(const boost::any& datum)
{
    boost::mutex::scoped_lock lock(mutex);
    activeStorage->push_back(TimeValueDatum(datum));
}

// ###### UTILITY ######

std::string Stats::summary(boost::chrono::microseconds window,
	                       StatValsMaker               statValsMaker,
	                       PostProcessorMaker          postProcessorMaker,
	                       FormatStringMaker           formatStringMaker)
{
    {
        boost::mutex::scoped_lock lock(mutex);
        
        // swap storages so that events can still be stored while we calculate the statistics
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        processingStorage  = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
    }
    
	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
	
	auto results = query(statValsMaker(window, now),
		                 postProcessorMaker(window, now));
    
    format::Dict dict;
    for (auto result : results)
    {
        dict(result.first, result.second);
    }
    dict("duration", formatDuration(window));

	std::string summary = format::fmt(formatStringMaker(window, now)) % dict;

	// Empty active storage to remove old data
	processingStorage->clear();
    
	return summary;
}

void Stats::autoSummaryLoop(boost::chrono::microseconds frequency,
							StatValsMaker               statValsMaker,
							PostProcessorMaker          postProcessorMaker,
							FormatStringMaker           formatStringMaker)
{
    auto summaryTime = boost::chrono::system_clock::now();

    while (true)
    {
        summaryTime += frequency;
        boost::this_thread::sleep_until(summaryTime);

        if (stopAutoSummary_)
        {
            return;
        }
		std::cout << summary(frequency, statValsMaker, postProcessorMaker, formatStringMaker);
    }
}

void Stats::autoSummary(boost::chrono::microseconds frequency,
						StatValsMaker               statValsMaker,
						PostProcessorMaker          postProcessorMaker,
						FormatStringMaker           formatStringMaker)
{
    stopAutoSummary_ = false;
	autoSummaryThread = boost::thread(boost::bind(&Stats::autoSummaryLoop, this, frequency, statValsMaker, postProcessorMaker, formatStringMaker));
}

void Stats::stopAutoSummary()
{
    stopAutoSummary_ = true;
}
