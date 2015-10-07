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

std::string Stats::summary(boost::chrono::microseconds window)
{
    {
        boost::mutex::scoped_lock lock(mutex);
        
        // swap storages so that events can still be stored while we calculate the statistics
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        processingStorage  = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
    }
    
	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();

	std::list<StatVal> statVals;

	statVals.insert(statVals.end(),
	{
		StatsUtils::cubemapsCount("cubemapsCount",
			window,
			now),
		StatsUtils::nalusBitSum("droppedNALUsBitSum",
			-1,
			StatsUtils::NALU::DROPPED,
			window,
			now),
		StatsUtils::nalusBitSum("addedNALUsBitSum",
			-1,
			StatsUtils::NALU::ADDED,
			window,
			now),
		StatsUtils::nalusBitSum("sentNALUsBitSum",
			-1,
			StatsUtils::NALU::SENT,
			window,
			now)
	});

	int faceCount = 12;
	for (int face = -1; face < faceCount; face++)
	{
		statVals.insert(statVals.end(),
		{
			StatsUtils::facesCount("facesCount" + std::to_string(face),
				face,
				window,
				now),
			StatsUtils::nalusCount("droppedNALUsCount" + std::to_string(face),
				face,
				StatsUtils::NALU::DROPPED,
				window,
				now),
			StatsUtils::nalusCount("addedNALUsCount" + std::to_string(face),
				face,
				StatsUtils::NALU::ADDED,
				window,
				now),
			StatsUtils::nalusCount("sentNALUsCount" + std::to_string(face),
				face,
				StatsUtils::NALU::SENT,
				window,
				now)
		});
	}

	auto results = query(statVals,
		[window, faceCount](std::map<std::string, double>& results)
		{
			unsigned long seconds = boost::chrono::duration_cast<boost::chrono::seconds>(window).count();

			results.insert(
			{
				{
					"naluDropRate",
					results["droppedNALUsCount-1"] / results["addedNALUsCount-1"]
				},
				{
					"fps",
					results["cubemapsCount"] / seconds
				},
				{
					"receivedNALUsBitS",
					(results["droppedNALUsBitSum"] + results["addedNALUsBitSum"]) / seconds
				},
				{
					"processedNALUsBitS",
					results["addedNALUsBitSum"] / seconds
				},
				{
					"sentNALUsBitS",
					results["sentNALUsBitSum"] / seconds
				},
			});

			for (int face = -1; face < faceCount; face++)
			{
				std::string faceStr = std::to_string(face);

				results.insert(
				{
					{
						"facesPS" + faceStr,
						results["facesCount" + faceStr] / seconds
					},
					{
						"receivedNALUsPS" + faceStr,
						(results["droppedNALUsCount" + faceStr] + results["addedNALUsCount" + faceStr]) / seconds
					},
					{
						"processedNALUsPS" + faceStr,
						results["addedNALUsCount" + faceStr] / seconds
					},
					{
						"sentNALUsPS" + faceStr,
						results["sentNALUsCount" + faceStr] / seconds
					},
				});
			}
	});
    
    std::stringstream stream;
    stream << "===============================================================================" << std::endl;
    stream << "Stats for last {duration}:" << std::endl;
    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "NALU drop rate: {naluDropRate:0.1f}" << std::endl;
    stream << "received NALUs/s: {receivedNALUsPS-1:0.1f}; {receivedNALUsBitS:0.1f} MBit/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "recvd NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t{receivedNALUsPS" << j * 6 + i << ":0.1f}";
        }
        stream << ";" << std::endl;
    }
    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "processed NALUs/s: {processedNALUsPS-1:0.1f}; {processedNALUsBitS:0.1f} MBit/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "prced NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t{processedNALUsPS" << j * 6 + i << ":0.1f}";
        }
        stream << ";" << std::endl;
    }

    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "sent NALUs/s: {sentNALUsPS-1:0.1f}; {sentNALUsBitS:0.1f} MBit/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "sent NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t{sentNALUsPS" << j * 6 + i << ":0.1f}";
        }
        stream << ";" << std::endl;
    }

	stream << "-------------------------------------------------------------------------------" << std::endl;
	stream << "cubemap face 0-5 (left ) fps:"; 
	for (int i = 0; i < 6; i++)
    {
        stream << "\t{facesPS" << i << ":0.1f}";
    }
    stream << ";" << std::endl;

    stream << "cubemap face 0-5 (right) fps:";
    for (int i = 6; i < 12; i++)
    {
        stream << "\t{facesPS" << i << ":0.1f}";
    }
    stream << ";" << std::endl;
    stream << "fps: {fps:0.1f}" << std::endl;
    
    format::Dict dict;
    for (auto result : results)
    {
        dict(result.first, result.second);
    }
    dict("duration", formatDuration(window));

	std::string summary = format::fmt(stream.str()) % dict;

	// Empty active storage to remove old data
	processingStorage->clear();
    
	return summary;
}

void Stats::autoSummaryLoop(boost::chrono::microseconds frequency)
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
        std::cout << summary(frequency);
    }
}

void Stats::autoSummary(boost::chrono::microseconds frequency)
{
    stopAutoSummary_ = false;
    autoSummaryThread = boost::thread(boost::bind(&Stats::autoSummaryLoop, this, frequency));
}

void Stats::stopAutoSummary()
{
    stopAutoSummary_ = true;
}
