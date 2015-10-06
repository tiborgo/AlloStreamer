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

double Stats::naluDropRate(bc::microseconds window,
                           boost::chrono::steady_clock::time_point now)
{
    auto results = query(
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
    
    return results["naluDropRate"];
}

double Stats::facesPS(int face,
                      boost::chrono::microseconds window,
                      boost::chrono::steady_clock::time_point now)
{
    auto results = query(
    {
        facesCount("facesCount",
                   face,
                   window,
                   now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["facePS"] = results["facesCount"] / bc::duration_cast<bc::seconds>(window).count();
    });

    return results["facePS"];
}

double Stats::fps(boost::chrono::microseconds window,
                  boost::chrono::steady_clock::time_point now)
{
    auto results = query(
    {
        cubemapsCount("cubemapsCount",
                      window,
                      now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["fps"] = results["cubemapsCount"] / bc::duration_cast<bc::seconds>(window).count();
    });
    
    return results["fps"];
}

double Stats::receivedNALUsPS(int face,
                              boost::chrono::microseconds window,
                              boost::chrono::steady_clock::time_point now)
{
    auto results = query(
    {
         nalusCount("droppedNALUsCount",
                    face,
                    NALU::DROPPED,
                    window,
                    now),
         nalusCount("addedNALUsCount",
                    face,
                    NALU::ADDED,
                    window,
                    now),
    },
    [window](std::map<std::string, double>& results)
    {
        results["receivedNALUsPS"] = (results["droppedNALUsCount"] + results["addedNALUsCount"]) /
            bc::duration_cast<bc::seconds>(window).count();
    });
    
    return results["receivedNALUsPS"];
}

double Stats::processedNALUsPS(int face,
                               boost::chrono::microseconds window,
                               boost::chrono::steady_clock::time_point now)
{
    auto results = query(
    {
        nalusCount("addedNALUsCount",
                   face,
                   NALU::ADDED,
                   window,
                   now),
    },
    [window](std::map<std::string, double>& results)
    {
        results["processedNALUsPS"] = results["addedNALUsCount"] / bc::duration_cast<bc::seconds>(window).count();
    });
    
    return results["processedNALUsPS"];
}

double Stats::sentNALUsPS(int face,
    boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
    auto results = query(
    {
        nalusCount("sentNALUsCount",
                   face,
                   NALU::SENT,
                   window,
                   now)
    },
    [window](std::map<std::string, double>& results)
    {
        results["sentNALUsS"] = results["sentNALUsCount"] / bc::duration_cast<bc::seconds>(window).count();
    });

    return results["sentNALUsS"];
}

double Stats::receivedNALUsBitRate(boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
    auto results = query(
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

    return results["receivedNALUsBitS"];
}

double Stats::processedNALUsBitRate(boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
    auto results = query(
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
    
    return results["processedNALUsBitS"];
}

double Stats::sentNALUsBitRate(boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
    auto results = query(
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
    
    return results["sentNALUsBitS"];
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
	double naluDropRateVal = naluDropRate(window, now);
	double receivedNALUsPSVal = receivedNALUsPS(-1, window, now);
	double processedNALUsPSVal = processedNALUsPS(-1, window, now);
	double sentNALUsPSVal = sentNALUsPS(-1, window, now);
	double receivedNALUsBitRateVal = receivedNALUsBitRate(window, now);
	double processedNALUsBitRateVal = processedNALUsBitRate(window, now);
	double sentNALUsBitRateVal = sentNALUsBitRate(window, now);
    int faceCount = 12;
    std::vector<double> facesPSVal(faceCount);
    std::vector<double> receivedNALUsPFPSVal(faceCount);
    std::vector<double> processedNALUsPFPSVal(faceCount);
    std::vector<double> sentNALUsPFPSVal(faceCount);

    for (int i = 0; i < faceCount; i++)
    {
		facesPSVal[i] = facesPS(i, window, now);
		receivedNALUsPFPSVal[i] = receivedNALUsPS(i, window, now);
		processedNALUsPFPSVal[i] = processedNALUsPS(i, window, now);
		sentNALUsPFPSVal[i] = sentNALUsPS(i, window, now);
    }
	double fpsVal = fps(window, now);
    
    /*format::Dict dict;
    for (int i = 0; i < 2; i++)
    {
        std::stringstream ss;
        ss << i;
        dict(ss.str().c_str(), i+5);
    }*/
    
    
    std::stringstream stream;
    //stream << format::fmt("The answer is {0} {1}") % dict << std::endl;
    stream << "===============================================================================" << std::endl;
    stream << "Stats for last " << formatDuration(window) << ": " << std::endl;
    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "NALU drop rate: " << naluDropRateVal << std::endl;
    stream << "received NALUs/s: " << receivedNALUsPSVal << "; " << to_human_readable_byte_count(receivedNALUsBitRateVal, true, false) << "/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "recvd NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t" << receivedNALUsPFPSVal[j * 6 + i];
        }
        stream << ";" << std::endl;
    }

    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "processed NALUs/s: " << processedNALUsPSVal << "; " << to_human_readable_byte_count(processedNALUsBitRateVal, true, false) << "/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "prced NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t" << processedNALUsPFPSVal[j * 6 + i];
        }
        stream << ";" << std::endl;
    }

    stream << "-------------------------------------------------------------------------------" << std::endl;
    stream << "sent NALUs/s: " << sentNALUsPSVal << "; " << to_human_readable_byte_count(sentNALUsBitRateVal, true, false) << "/s;" << std::endl;
    for (int j = 0; j < (std::min) (2, faceCount); j++)
    {
        stream << "sent NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
        for (int i = 0; i < (std::min) (6, faceCount - j * 6); i++)
        {
            stream << "\t" << sentNALUsPFPSVal[j * 6 + i];
        }
        stream << ";" << std::endl;
    }

	stream << "-------------------------------------------------------------------------------" << std::endl;
	stream << "cubemap face 0-5 (left ) fps:"; 
	for (int i = 0; i < 6; i++)
    {
        stream << "\t" << facesPSVal[i];
    }
    stream << ";" << std::endl;

    stream << "cubemap face 0-5 (right) fps:";
    for (int i = 6; i < 12; i++)
    {
        stream << "\t" << facesPSVal[i];
    }
    stream << ";" << std::endl;
    stream << "fps: " << fpsVal << std::endl;

    std::string result = stream.str();
    return result;
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
