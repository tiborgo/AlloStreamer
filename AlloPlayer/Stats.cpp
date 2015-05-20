#include <boost/chrono/system_clocks.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/thread/thread.hpp>

#include "Stats.h"

namespace bc = boost::chrono;
namespace ba = boost::accumulators;
namespace bad = boost::adaptors;
namespace bf = boost::fusion;

template <typename ValueType>
Stats::TimeValueDatum<ValueType>::TimeValueDatum(ValueType value)
:
timeSinceEpoch(bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch())),
value(value)
{
}

template <typename ValueType>
bool Stats::isInTime(Stats::TimeValueDatum<ValueType> datum,
    bc::microseconds window,
    bc::microseconds nowSinceEpoch)
{
    return (nowSinceEpoch - datum.timeSinceEpoch) < window;
}

template <typename Features, typename ValueType>
ba::accumulator_set<Stats::TimeValueDatum<ValueType>, Features> Stats::filterTime(
    std::vector<TimeValueDatum<ValueType> >& data,
    bc::microseconds window,
    bc::microseconds nowSinceEpoch)
{
    ba::accumulator_set<TimeValueDatum<ValueType>, Features> acc;
    auto filteredData = data | bad::filtered(boost::bind(&Stats::isInTime<ValueType>, this, _1, window, nowSinceEpoch));
    std::for_each(filteredData.begin(), filteredData.end(), boost::bind<void>(boost::ref(acc), _1));
    return acc;
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
    return format.substr(0, format.size()-1);
}

// ###### EVENTS ######

void Stats::droppedNALU(int type)
{
    boost::mutex::scoped_lock lock(mutex);
    droppedNALUs.push_back(TimeValueDatum<int>(type));
}

void Stats::addedNALU(int type)
{
    boost::mutex::scoped_lock lock(mutex);
    addedNALUs.push_back(TimeValueDatum<int>(type));
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
}

// ###### STATISTICAL VALUES ######

double Stats::naluDropRate(bc::microseconds window, bc::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    if (nowSinceEpoch.count() == 0)
    {
        nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    }
    
    auto accDropped = filterTime<ba::features<ba::tag::count> >(droppedNALUs, window, nowSinceEpoch);
    auto accAdded = filterTime<ba::features<ba::tag::count> >(addedNALUs, window, nowSinceEpoch);
    
    return (double)ba::count(accDropped) / (double)ba::count(accAdded);
}

double Stats::cubemapFacesPS(int face,
    boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    if (nowSinceEpoch.count() == 0)
    {
        nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    }
    
    auto accDisplayedCubemapFaces = filterTime<ba::features<ba::tag::count> >(displayedCubemapFaces, window, nowSinceEpoch);
    
    return (double)ba::count(accDisplayedCubemapFaces) / bc::duration_cast<bc::seconds>(window).count() / 6;
}

double Stats::fps(boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    if (nowSinceEpoch.count() == 0)
    {
        nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    }
    
    auto accDisplayedFrames = filterTime<ba::features<ba::tag::count> >(displayedFrames, window, nowSinceEpoch);
    
    return (double)ba::count(accDisplayedFrames) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::receivedNALUsPS(boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    if (nowSinceEpoch.count() == 0)
    {
        nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    }
    
    auto accDropped = filterTime<ba::features<ba::tag::count> >(droppedNALUs, window, nowSinceEpoch);
    auto accAdded = filterTime<ba::features<ba::tag::count> >(addedNALUs, window, nowSinceEpoch);
    
    return ((double)ba::count(accDropped) + ba::count(accAdded)) / bc::duration_cast<bc::seconds>(window).count();
}

double Stats::processedNALUsPS(boost::chrono::microseconds window,
    boost::chrono::microseconds nowSinceEpoch)
{
    boost::mutex::scoped_lock lock(mutex);
    
    if (nowSinceEpoch.count() == 0)
    {
        nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    }
    
    auto accAdded = filterTime<ba::features<ba::tag::count> >(addedNALUs, window, nowSinceEpoch);
    
    return (double)ba::count(accAdded) / bc::duration_cast<bc::seconds>(window).count();
}

// ###### UTILITY ######

std::string Stats::summary(bc::microseconds window)
{
    bc::microseconds nowSinceEpoch = bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    double receivedNALUsPSVal = receivedNALUsPS(window, nowSinceEpoch);
    double processedNALUsPSVal = processedNALUsPS(window, nowSinceEpoch);
    double cubemapFacesPSVal = cubemapFacesPS(0, window, nowSinceEpoch);
    double fpsVal = fps(window, nowSinceEpoch);
    
    std::stringstream stream;
    stream << "Stats for last " << formatDuration(window) << ": ";
    stream << "received NALUs/s: " << receivedNALUsPSVal << "; processed NALUs/s: " << processedNALUsPSVal;
    stream << "; cubemap faces/s: " << cubemapFacesPSVal << "; fps: " << fpsVal << std::endl;
    
    std::string result = stream.str();
    return result;
}

void Stats::autoSummaryLoop(boost::chrono::microseconds frequency)
{
    while (true)
    {
        std::cout << summary(frequency);
        boost::this_thread::sleep(boost::posix_time::microseconds(frequency.count()));
    }
}

void Stats::autoSummary(boost::chrono::microseconds frequency)
{
    autoSummaryThread = boost::thread(boost::bind(&Stats::autoSummaryLoop, this, frequency));
}