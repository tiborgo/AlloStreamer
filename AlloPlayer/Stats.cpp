#include <boost/chrono/system_clocks.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

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

// ###### EVENTS ######

void Stats::droppedNALU(int type)
{
    droppedNALUs.push_back(TimeValueDatum<int>(type));
}

void Stats::addedNALU(int type)
{
    addedNALUs.push_back(TimeValueDatum<int>(type));
}

// ###### STATISTICAL VALUES ######

double Stats::naluDropRate(bc::microseconds window)
{
    bc::microseconds nowSinceEpoch =
        bc::duration_cast<bc::microseconds>(bc::system_clock::now().time_since_epoch());
    
    auto accDropped = filterTime<ba::features<ba::tag::count> >(droppedNALUs, window, nowSinceEpoch);
    auto accAdded = filterTime<ba::features<ba::tag::count> >(addedNALUs, window, nowSinceEpoch);
    
    return (double)ba::count(accDropped) / (double)ba::count(accAdded);
}