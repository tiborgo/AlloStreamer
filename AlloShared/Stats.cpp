#include <boost/chrono/system_clocks.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>

#include "to_human_readable_byte_count.hpp"
#include "Stats.hpp"

namespace bc = boost::chrono;
namespace ba = boost::accumulators;
namespace bf = boost::fusion;

boost::function<bool (Stats::TimeValueDatum&)> Stats::timeFilter(bc::microseconds window,
                                                                 bc::microseconds nowSinceEpoch)
{
    return [window, nowSinceEpoch](Stats::TimeValueDatum& datum)
    {
        return (nowSinceEpoch - datum.timeSinceEpoch) < window;
    };
}

boost::function<double (Stats::TimeValueDatum&, void*&)> Stats::sumAcc(boost::function<double (TimeValueDatum&)> accessor)
{
    /*struct SumAccHelper
    {
        double sum;
        boost::function<double (TimeValueDatum&)> accessor;
        SumAccHelper(boost::function<double (TimeValueDatum&)> accessor) : sum(0.0), accessor(accessor) {}
        double operator()(TimeValueDatum& datum)
        {
            sum += accessor(datum);
            return sum;
        }
    };
    return SumAccHelper(accessor);*/
    
    return [accessor](Stats::TimeValueDatum& datum, void*& state)
    {
        if (!state)
        {
            state = new ba::accumulator_set<double, ba::features<ba::tag::sum> >();
        }
        auto acc = (ba::accumulator_set<double, ba::features<ba::tag::sum> >*)state;
        (*acc)(accessor(datum));
        return ba::sum(*acc);
    };
}

std::vector<double> Stats::query(const std::vector<boost::function<bool   (TimeValueDatum&)> >& filters,
                                 const std::vector<boost::function<double (TimeValueDatum&, void*&)> >& accumulators)
{
    std::vector<double> accumulations(accumulators.size(), 0.0);
    std::vector<void*>  states(accumulators.size(), nullptr);
    
    for (TimeValueDatum& datum : storage)
    {
        bool passed = true;
        for (auto filter : filters)
        {
            if (!filter(datum))
            {
                passed = false;
                break;
            }
        }
        if (passed)
        {
            size_t i = 0;
            for (auto accumulator : accumulators)
            {
                accumulations[i] = accumulator(datum, states[i]);
                i++;
            }
        }
    }
    
    // Delete states
    for (void* state : states)
    {
        delete state;
    }
    
    return accumulations;
}

void Stats::store(const boost::any& datum)
{
    storage.push_back(TimeValueDatum(datum));
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
    return format.substr(0, format.size()-1);
}

// ###### EVENTS ######

/*void Stats::droppedNALU(int type, size_t size, int face)
{
    boost::mutex::scoped_lock lock(mutex);
    droppedNALUs.push_back(TimeValueDatum<NALU>(NALU(type, size, face)));
}

void Stats::addedNALU(int type, size_t size, int face)
{
    boost::mutex::scoped_lock lock(mutex);
	addedNALUs.push_back(TimeValueDatum<NALU>(NALU(type, size, face)));
}

void Stats::sentNALU(int type, size_t size, int face)
{
	boost::mutex::scoped_lock lock(mutex);
	sentNALUs.push_back(TimeValueDatum<NALU>(NALU(type, size, face)));
}

void Stats::displayedCubemapFace(int face)
{
    boost::mutex::scoped_lock lock(mutex);
    displayedCubemapFaces.push_back(TimeValueDatum<int>(face));
}

void Stats::displayedFrame()
{
    boost::mutex::scoped_lock lock(mutex);
    displayedFrames.push_back(TimeValueDatum<int>(0));
}*/

// ###### STATISTICAL VALUES ######

/*double Stats::naluDropRate(bc::microseconds window, bc::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
	auto accDropped = filter<ba::features<ba::tag::count>, NALU, NALU>(droppedNALUs,
	                                                                   { timeFilter<NALU>(window,
	                                                                                      nowSinceEpoch) },
																       [](NALU nalu) { return nalu; });
    auto accAdded = filter<ba::features<ba::tag::count>, NALU, NALU>(addedNALUs,
	                                                                 { timeFilter<NALU>(window,
	                                                                                    nowSinceEpoch) },
																	   [](NALU nalu) { return nalu; });
    
    return (double)ba::count(accDropped) / (double)ba::count(accAdded);
}

double Stats::facesPS(int face,
    boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    auto accDisplayedCubemapFaces = filter<ba::features<ba::tag::count>, int, int>(displayedCubemapFaces,
	                                                                          { timeFilter<int>(window, nowSinceEpoch),
																			    [face](Stats::TimeValueDatum<int> datum) { return datum.value == face; } },
																				[](int face) { return face; });
    
    return (double)ba::count(accDisplayedCubemapFaces) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::fps(boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    auto accDisplayedFrames = filter<ba::features<ba::tag::count>, int, int>(displayedFrames,
	                                                                   { timeFilter<int>(window,
	                                                                     nowSinceEpoch) },
																		 [](int fps) { return fps; });
    
    return (double)ba::count(accDisplayedFrames) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::receivedNALUsPS(int face,
    boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);

	auto accDropped = filter<ba::features<ba::tag::count>, NALU, NALU>(droppedNALUs,
	                                                             { timeFilter<NALU>(window,
	                                                                                nowSinceEpoch),
																   faceFilter(face) },
																 [](NALU nalu) { return nalu; });
	auto accAdded = filter<ba::features<ba::tag::count>, NALU, NALU>(addedNALUs,
	                                                           { timeFilter<NALU>(window,
	                                                                              nowSinceEpoch),
															     faceFilter(face) },
																[](NALU nalu) { return nalu; });
    
	return ((double)ba::count(accDropped) + ba::count(accAdded)) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::processedNALUsPS(int face,
	                           boost::chrono::microseconds window,
                               boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
	auto accAdded = filter<ba::features<ba::tag::count>, NALU, NALU>(addedNALUs,
	                                                           { timeFilter<NALU>(window,
	                                                                              nowSinceEpoch),
	                                                             faceFilter(face) },
									                           [](NALU nalu) { return nalu; });
    
    return (double)ba::count(accAdded) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::sentNALUsPS(int face,
	                      boost::chrono::microseconds window,
	                      boost::chrono::microseconds nowSinceEpoch)
{
	boost::mutex::scoped_lock lock(mutex);

	auto countSent = filter<ba::features<ba::tag::count>, NALU, NALU>(sentNALUs,
	                                                                  { timeFilter<NALU>(window,
	                                                                                     nowSinceEpoch),
																		faceFilter(face) },
	                                                                  [](NALU nalu) { return nalu; });

	return (double)ba::count(countSent) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::receivedNALUsBitRate(boost::chrono::microseconds window,
	boost::chrono::microseconds nowSinceEpoch)
{
	boost::mutex::scoped_lock lock(mutex);

	auto sumDropped = filter<ba::features<ba::tag::sum>, NALU, int>(droppedNALUs,
	{ timeFilter<NALU>(window,
	nowSinceEpoch) },
	[](NALU nalu) { return nalu.size; });

	auto sumAdded = filter<ba::features<ba::tag::sum>, NALU, int>(addedNALUs,
	{ timeFilter<NALU>(window,
	nowSinceEpoch) },
	[](NALU nalu) { return nalu.size; });

	return ((double)ba::sum(sumDropped) + ba::sum(sumAdded)) * 8. / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::processedNALUsBitRate(boost::chrono::microseconds window,
	boost::chrono::microseconds nowSinceEpoch)
{
	boost::mutex::scoped_lock lock(mutex);

	auto accSum = filter<ba::features<ba::tag::sum>, NALU, int>(addedNALUs,
	                                                           { timeFilter<NALU>(window,
	                                                                              nowSinceEpoch) },
																				  [](NALU nalu) { return nalu.size; });

	return (double)ba::sum(accSum) * 8. / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::sentNALUsBitRate(boost::chrono::microseconds window,
	                           boost::chrono::microseconds nowSinceEpoch)
{
	boost::mutex::scoped_lock lock(mutex);

	auto sentSum = filter<ba::features<ba::tag::sum>, NALU, int>(sentNALUs,
	                                                             { timeFilter<NALU>(window,
	                                                                                nowSinceEpoch) },
	                                                             [](NALU nalu) { return nalu.size; });

	return (double)ba::sum(sentSum) * 8. / bc::duration_cast<bc::seconds>(window).count();
}*/

// ###### UTILITY ######

/*std::string Stats::summary(std::initializer_list<std::initializer_list<boost::function<bool   (TimeValueDatum&)> > >     filters,
                           std::initializer_list<std::initializer_list<boost::function<double (TimeValueDatum&)> > >     accumulators,
                           const std::string&                                                                                   format,
                           boost::chrono::microseconds                                                               window,
                           boost::chrono::microseconds                                                               nowSinceEpoch)
{
    std::vector<boost::function<bool   (TimeValueDatum&)> > filtersVector;
    std::vector<boost::function<double (TimeValueDatum&)> > accumulatorsVector;
    std::copy(filters.begin(), filters.end(), filtersVector.end());
    std::copy(accumulators.begin(), accumulators.end(), accumulatorsVector.end());
    summary(filtersVector, accumulatorsVector, format, window, nowSinceEpoch);
}*/

std::string Stats::summary(std::vector<std::vector<boost::function<bool   (TimeValueDatum&)> > >      filters,
                           std::vector<std::vector<boost::function<double (TimeValueDatum&)> > >      accumulators,
                           const std::string&                                                         format,
                           std::vector<boost::function<std::string (double)> >                        formatters,
                           boost::chrono::microseconds                                                window,
                           boost::chrono::microseconds                                                nowSinceEpoch)
{
    std::vector<double> accumulations;
    
    assert(filters.size() == accumulators.size());
    
    auto filterIt = filters.begin();
    auto accumulatorIt = accumulators.begin();
    
    for (;
     filterIt != filters.end() && accumulatorIt != accumulators.end();
     ++filterIt, ++accumulatorIt)
    {
        /*std::vector<boost::function<bool   (TimeValueDatum&)> > filtersVector;
         std::vector<boost::function<double (TimeValueDatum&)> > accumulatorsVector;
         filtersVector.push_back(timeFilter(window, nowSinceEpoch));
         for (auto filter : pair.first)
         {
         filtersVector.push_back(filter);
         }
         for (auto accumulator : pair.second)
         {
         accumulatorsVector.push_back(accumulator);
         }*/
        
        std::vector<double> pairAccumulations = query(*filterIt,
                                                      *accumulatorIt);
        
        /*std::vector<double> pairAccumulations = query(filters,
                                                      accumulators);*/
        
        accumulations.insert(accumulations.end(), pairAccumulations.begin(), pairAccumulations.end());
    }
    
    
    /*double receivedNALUsPSVal = receivedNALUsPS(-1, window, nowSinceEpoch);
     double processedNALUsPSVal = processedNALUsPS(-1, window, nowSinceEpoch);
     double sentNALUsPSVal = sentNALUsPS(-1, window, nowSinceEpoch);
     double receivedNALUsBitRateVal = receivedNALUsBitRate(window, nowSinceEpoch);
     double processedNALUsBitRateVal = processedNALUsBitRate(window, nowSinceEpoch);
     double sentNALUsBitRateVal = sentNALUsBitRate(window, nowSinceEpoch);
     int faceCount = 12;
     std::vector<double> facesPSVal(faceCount);
     std::vector<double> receivedNALUsPFPSVal(faceCount);
     std::vector<double> processedNALUsPFPSVal(faceCount);
     std::vector<double> sentNALUsPFPSVal(faceCount);
     for (int i = 0; i < faceCount; i++)
     {
     facesPSVal[i]            = facesPS(i, window, nowSinceEpoch);
     receivedNALUsPFPSVal[i]  = receivedNALUsPS(i, window, nowSinceEpoch);
     processedNALUsPFPSVal[i] = processedNALUsPS(i, window, nowSinceEpoch);
     sentNALUsPFPSVal[i]      = sentNALUsPS(i, window, nowSinceEpoch);
     }
     double fpsVal = fps(window, nowSinceEpoch);*/
    
    assert(accumulations.size() == formatters.size());
    
    boost::format formatter(format.c_str());
    auto formattersIt = formatters.begin();
    auto accumulationsIt = accumulations.begin();
    
    for (;
         formattersIt != formatters.end() && accumulationsIt != accumulations.end();
         ++formattersIt, ++accumulationsIt)
    {
        formatter % (*formattersIt)(*accumulationsIt);
    }
    
    return formatter.str();
    
    /*std::stringstream stream;
     stream << "===============================================================================" << std::endl;
     stream << "Stats for last " << formatDuration(window) << ": " << std::endl;
     
     stream << "-------------------------------------------------------------------------------" << std::endl;
     stream << "received NALUs/s: " << receivedNALUsPSVal << "; " << to_human_readable_byte_count(receivedNALUsBitRateVal, true, false) << "/s;" << std::endl;
     for (int j = 0; j < (std::min)(2, faceCount); j++)
     {
     stream << "recvd NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
     for (int i = 0; i < (std::min)(6, faceCount - j * 6); i++)
     {
     stream << "\t" << receivedNALUsPFPSVal[j*6+i];
     }
     stream << ";" << std::endl;
     }
     
     stream << "-------------------------------------------------------------------------------" << std::endl;
     stream << "processed NALUs/s: " << processedNALUsPSVal << "; " << to_human_readable_byte_count(processedNALUsBitRateVal, true, false) << "/s;" << std::endl;
     for (int j = 0; j < (std::min)(2, faceCount); j++)
     {
     stream << "prced NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
     for (int i = 0; i < (std::min)(6, faceCount - j * 6); i++)
     {
     stream << "\t" << processedNALUsPFPSVal[j * 6 + i];
     }
     stream << ";" << std::endl;
     }
     
     stream << "-------------------------------------------------------------------------------" << std::endl;
     stream << "sent NALUs/s: " << sentNALUsPSVal << "; " << to_human_readable_byte_count(sentNALUsBitRateVal, true, false) << "/s;" << std::endl;
     for (int j = 0; j < (std::min)(2, faceCount); j++)
     {
     stream << "sent NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
     for (int i = 0; i < (std::min)(6, faceCount - j * 6); i++)
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
     return result;*/
}

void Stats::autoSummaryLoop(std::vector<std::vector<boost::function<bool   (TimeValueDatum&)> > >     filters,
                            std::vector<std::vector<boost::function<double (TimeValueDatum&)> > >     accumulators,
                            std::string                                                                                   format,
                            std::vector<boost::function<std::string (double)> >                       formatters,
                            boost::chrono::microseconds                                                                          interval)
{
    auto summaryTime = boost::chrono::system_clock::now();
    
    while (true)
    {
        summaryTime += interval;
        boost::this_thread::sleep_until(summaryTime);
        
        if (stopAutoSummary_)
        {
            return;
        }
        std::cout << summary(filters,
                             accumulators,
                             format,
                             formatters,
                             interval,
                             boost::chrono::duration_cast<boost::chrono::microseconds>(summaryTime.time_since_epoch()));
    }
}

void Stats::startAutoSummary(std::initializer_list<std::initializer_list<boost::function<bool   (TimeValueDatum&)> > >     filters,
                             std::initializer_list<std::initializer_list<boost::function<double (TimeValueDatum&)> > >     accumulators,
                             const std::string&                                                                                   format,
                             std::initializer_list<boost::function<std::string (double)> >                       formatters,
                             boost::chrono::microseconds                                                                          interval)
{
    std::vector<std::vector<boost::function<bool (TimeValueDatum&)> > > filtersVector(filters.size());
    for (auto filter : filters)
    {
        std::vector<boost::function<bool (TimeValueDatum&)> > filterVector(filter.size());
        std::copy(filter.begin(), filter.end(), filterVector.begin());
        filtersVector.push_back(filterVector);
    }
    
    std::vector<std::vector<boost::function<double (TimeValueDatum&)> > > accumulatorsVector(accumulators.size());
    for (auto accumulator : accumulators)
    {
        std::vector<boost::function<double (TimeValueDatum&)> > accumulatorVector(accumulator.size());
        std::copy(accumulator.begin(), accumulator.end(), accumulatorVector.begin());
        accumulatorsVector.push_back(accumulatorVector);
    }
    
    std::vector<boost::function<std::string (double)> > formattersVector(formatters.size());
    std::copy(formatters.begin(), formatters.end(), formattersVector.begin());
    
    startAutoSummary(filtersVector, accumulatorsVector, format, formattersVector, interval);
}

void Stats::startAutoSummary(std::vector<std::vector<boost::function<bool   (TimeValueDatum&)> > >     filters,
                             std::vector<std::vector<boost::function<double (TimeValueDatum&)> > >     accumulators,
                             const std::string&                                                                                   format,
                             std::vector<boost::function<std::string (double)> >                       formatters,
                             boost::chrono::microseconds                                                                          interval)
{
    stopAutoSummary_ = false;
    autoSummaryThread = boost::thread(boost::bind(&Stats::autoSummaryLoop, this, filters, accumulators, format, formatters, interval));
}

void Stats::stopAutoSummary()
{
	stopAutoSummary_ = true;
}