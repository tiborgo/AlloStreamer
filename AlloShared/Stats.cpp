#include <boost/chrono/system_clocks.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/thread/thread.hpp>

#include "format.hpp"
#include "to_human_readable_byte_count.hpp"
#include "Stats.hpp"

namespace bc = boost::chrono;
namespace ba = boost::accumulators;
namespace bad = boost::adaptors;
namespace bf = boost::fusion;

Stats::Stats()
    :
    activeStorage(&storage1),
    processingStorage(&storage2)
{
    
}

boost::function<bool (Stats::TimeValueDatum)> Stats::timeFilter(boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
    return [window, now](Stats::TimeValueDatum datum)
    {
		return (now - datum.time) < window;
    };
}

boost::function<bool (Stats::TimeValueDatum)> Stats::typeFilter(const std::type_info& type)
{
    return [&type](TimeValueDatum datum)
    {
        return datum.value.type() == type;
    };
}

boost::function<bool (Stats::TimeValueDatum)> Stats::naluFaceFilter(int face)
{
	return [face](TimeValueDatum datum)
	{
        return (face == -1) ? true : boost::any_cast<NALU>(datum.value).face == face;
	};
}

boost::function<bool (Stats::TimeValueDatum)> Stats::naluStatusFilter(NALU::Status status)
{
    return [status](TimeValueDatum datum)
    {
        return boost::any_cast<NALU>(datum.value).status == status;
    };
}

boost::function<bool (Stats::TimeValueDatum)> Stats::andFilter(std::initializer_list<boost::function<bool (TimeValueDatum)> > filtersList)
{
    // Make filters permanent
    std::vector<boost::function<bool (TimeValueDatum)> > filters(filtersList.size());
    std::copy(filtersList.begin(), filtersList.end(), filters.begin());
    
    return [filters](TimeValueDatum datum)
    {
        for (auto filter : filters)
        {
            if (!filter(datum))
            {
                return false;
            }
        }
        return true;
    };
}

std::map<std::string, double> Stats::query(std::initializer_list<StatVal>                         statVals,
                                           boost::function<void (std::map<std::string, double>&)> postCalculator)
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
    auto faesIt     = faes.begin();
    auto statValsIt = statVals.begin();
    for (; faesIt != faes.end() && statValsIt != statVals.end(); ++faesIt, ++statValsIt)
    {
        results[statValsIt->name] = faesIt->second();
    }
    
    // Make post calculations
    postCalculator(results);
    
    return results;
}

bc::microseconds Stats::nowSinceEpoch()
{
    return bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
}

std::string Stats::formatDuration(bc::microseconds duration)
{
    std::stringstream result;
    bc::microseconds remainder(duration);

    bc::hours h = bc::duration_cast<bc::hours>(remainder);

    if (h.count() > 0)
    {
        result << h.count() << "h ";
        remainder -= h;
    }

    bc::minutes m = bc::duration_cast<bc::minutes>(remainder);
    if (m.count() > 0)
    {
        result << m.count() << "m ";
        remainder -= m;
    }

    bc::seconds s = bc::duration_cast<bc::seconds>(remainder);
    if (s.count() > 0)
    {
        result << s.count() << "s ";
        remainder -= s;
    }

    bc::milliseconds ms = bc::duration_cast<bc::milliseconds>(remainder);
    if (ms.count() > 0)
    {
        result << ms.count() << "ms ";
        remainder -= ms;
    }

    bc::microseconds us = bc::duration_cast<bc::microseconds>(remainder);
    if (us.count() > 0)
    {
        result << us.count() << "µs ";
        remainder -= us;
    }

    std::string format = result.str();
    return format.substr(0, format.size() - 1);
}

// ###### STAT VALS ######

Stats::StatVal Stats::nalusBitSum       (const std::string&                      name,
                                         int                                     face,
                                         NALU::Status                            status,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
    return StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(NALU)),
        naluStatusFilter(status),
        naluFaceFilter(face)
    }),
    [](TimeValueDatum datum)
    {
        return boost::any_cast<NALU>(datum.value).size * 8.0;
    },
    ba::tag::sum(),
    name);
}

Stats::StatVal Stats::nalusCount        (const std::string&                      name,
                                         int                                     face,
                                         NALU::Status                            status,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
    return StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(NALU)),
        naluStatusFilter(status),
        naluFaceFilter(face)
    }),
    [](TimeValueDatum datum)
    {
        return 0.0;
    },
    ba::tag::count(),
    name);
}

Stats::StatVal Stats::cubemapsCount     (const std::string&                      name,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
    return StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(Cubemap))
    }),
    [](TimeValueDatum datum)
    {
        return 0.0;
    },
    ba::tag::count(),
    name);
}

Stats::StatVal Stats::facesCount        (const std::string&                      name,
                                         int                                     face,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
    return StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(CubemapFace)),
        [face](TimeValueDatum datum)
        {
            return (face == -1) ? true : boost::any_cast<CubemapFace>(datum.value).face == face;
        }
    }),
    [](TimeValueDatum datum)
    {
        return 0.0;
    },
    ba::tag::count(),
    name);
}


// ###### EVENTS ######

void Stats::store(const boost::any& datum)
{
    boost::mutex::scoped_lock lock(mutex);
    activeStorage->push_back(TimeValueDatum(datum));
}

// ###### STATISTICAL VALUES ######

std::map<std::string, double> Stats::naluDropRate(bc::microseconds window,
                                                  boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusCount("droppedNALUsCount",
                   -1,
                   NALU::DROPPED,
                   window,
                   now),
        nalusCount("addedNALUsCount",
                   -1,
                   NALU::ADDED,
                   window,
                   now),
    },
    [](std::map<std::string, double>& results)
    {
        results["naluDropRate"] = results["droppedNALUsCount"] / results["addedNALUsCount"];
    });
}

std::map<std::string, double> Stats::facesPS(int face,
                                             boost::chrono::microseconds window,
                                             boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        facesCount("facesCount" + std::to_string(face),
                   face,
                   window,
                   now)
    },
    [window, face](std::map<std::string, double>& results)
    {
        results["facesPS" + std::to_string(face)] = results["facesCount" + std::to_string(face)] /
            bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::fps(boost::chrono::microseconds window,
                                         boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        cubemapsCount("cubemapsCount",
                      window,
                      now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["fps"] = results["cubemapsCount"] / bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::receivedNALUsPS(int face,
                                                     boost::chrono::microseconds window,
                                                     boost::chrono::steady_clock::time_point now)
{
    return query(
    {
         nalusCount("droppedNALUsCount" + std::to_string(face),
                    face,
                    NALU::DROPPED,
                    window,
                    now),
         nalusCount("addedNALUsCount" + std::to_string(face),
                    face,
                    NALU::ADDED,
                    window,
                    now),
    },
    [window, face](std::map<std::string, double>& results)
    {
        results["receivedNALUsPS" + std::to_string(face)] = (results["droppedNALUsCount" + std::to_string(face)] +
                                                             results["addedNALUsCount" + std::to_string(face)]) /
            bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::processedNALUsPS(int face,
                                                      boost::chrono::microseconds window,
                                                      boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusCount("addedNALUsCount" + std::to_string(face),
                   face,
                   NALU::ADDED,
                   window,
                   now),
    },
    [window, face](std::map<std::string, double>& results)
    {
        results["processedNALUsPS" + std::to_string(face)] = results["addedNALUsCount" + std::to_string(face)] /
            bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::sentNALUsPS(int face,
                                                 boost::chrono::microseconds window,
                                                 boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusCount("sentNALUsCount" + std::to_string(face),
                   face,
                   NALU::SENT,
                   window,
                   now)
    },
    [window, face](std::map<std::string, double>& results)
    {
        results["sentNALUsPS" + std::to_string(face)] = results["sentNALUsCount" + std::to_string(face)] /
            bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::receivedNALUsBitRate(boost::chrono::microseconds window,
                                                          boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusBitSum("droppedNALUsBitSum",
                    -1,
                    NALU::DROPPED,
                    window,
                    now),
        nalusBitSum("addedNALUsBitSum",
                    -1,
                    NALU::ADDED,
                    window,
                    now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["receivedNALUsBitS"] = (results["droppedNALUsBitSum"] + results["addedNALUsBitSum"]) /
            bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::processedNALUsBitRate(boost::chrono::microseconds window,
                                                           boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusBitSum("addedNALUsBitSum",
                    -1,
                    NALU::ADDED,
                    window,
                    now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["processedNALUsBitS"] = results["addedNALUsBitSum"] / bc::duration_cast<bc::seconds>(window).count();
    });
}

std::map<std::string, double> Stats::sentNALUsBitRate(boost::chrono::microseconds window,
                                                      boost::chrono::steady_clock::time_point now)
{
    return query(
    {
        nalusBitSum("sentNALUsBitSum",
                    -1,
                    NALU::SENT,
                    window,
                    now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["sentNALUsBitS"] = results["sentNALUsBitSum"] / bc::duration_cast<bc::seconds>(window).count();
    });
}

// ###### UTILITY ######

std::string Stats::summary(bc::microseconds window)
{
    {
        boost::mutex::scoped_lock lock(mutex);
        
        // swap storages so that events can still be stored while we calculate the statistics
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        processingStorage  = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        activeStorage      = (std::list<TimeValueDatum>*)((uintptr_t)activeStorage ^ (uintptr_t)processingStorage);
        
        // Empty active storage to remove old data
        activeStorage->clear();
    }
    
	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
    
    std::map<std::string, double> results;
    std::map<std::string, double> subresults;
    
	subresults = naluDropRate(window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = receivedNALUsPS(-1, window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = processedNALUsPS(-1, window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = sentNALUsPS(-1, window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = receivedNALUsBitRate(window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = processedNALUsBitRate(window, now);
    results.insert(subresults.begin(), subresults.end());
	subresults = sentNALUsBitRate(window, now);
    results.insert(subresults.begin(), subresults.end());
    
    int faceCount = 12;
    for (int i = 0; i < faceCount; i++)
    {
		subresults = facesPS(i, window, now);
        results.insert(subresults.begin(), subresults.end());
		subresults = receivedNALUsPS(i, window, now);
        results.insert(subresults.begin(), subresults.end());
		subresults = processedNALUsPS(i, window, now);
        results.insert(subresults.begin(), subresults.end());
		subresults = sentNALUsPS(i, window, now);
        results.insert(subresults.begin(), subresults.end());
    }
	subresults = fps(window, now);
    results.insert(subresults.begin(), subresults.end());
    
    
    std::stringstream stream;
    stream << "===============================================================================" << std::endl;
    stream << "Stats for last {duration}:" << std::endl;
    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "NALU drop rate: {naluDropRate:0.1f}" << std::endl;
    stream << "received NALUs/s: {receivedNALUsPS-1:0.1f}; {receivedNALUsBitS:0.1f} Bit/s;" << std::endl;
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
    stream << "processed NALUs/s: {processedNALUsPS-1:0.1f}; {processedNALUsBitS:0.1f} Bit/s;" << std::endl;
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
    stream << "sent NALUs/s: {sentNALUsPS-1:0.1f}; {sentNALUsBitS:0.1f} Bit/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "sent NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t{processedNALUsPS" << j * 6 + i << ":0.1f}";
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
    
    return format::fmt(stream.str()) % dict;
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
