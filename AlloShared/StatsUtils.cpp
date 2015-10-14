#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/sum.hpp>

#include "StatsUtils.hpp"

// ###### STAT VALS ######

Stats::StatVal StatsUtils::nalusBitSum  (const std::string&                      name,
                                         int                                     face,
                                         NALU::Status                            status,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
	return Stats::StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(NALU)),
        naluStatusFilter(status),
        naluFaceFilter(face)
    }),
	[](Stats::TimeValueDatum datum)
    {
        return boost::any_cast<NALU>(datum.value).size * 8.0 / 1000000.0;
    },
    boost::accumulators::tag::sum(),
    name);
}

Stats::StatVal StatsUtils::nalusCount   (const std::string&                      name,
                                         int                                     face,
                                         NALU::Status                            status,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
	return Stats::StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(NALU)),
        naluStatusFilter(status),
        naluFaceFilter(face)
    }),
	[](Stats::TimeValueDatum datum)
    {
        return 0.0;
    },
	boost::accumulators::tag::count(),
    name);
}

Stats::StatVal StatsUtils::cubemapsCount(const std::string&                      name,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
	return Stats::StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(Cubemap))
    }),
	[](Stats::TimeValueDatum datum)
    {
        return 0.0;
    },
	boost::accumulators::tag::count(),
    name);
}

Stats::StatVal StatsUtils::facesCount   (const std::string&                      name,
                                         int                                     face,
                                         boost::chrono::microseconds             window,
                                         boost::chrono::steady_clock::time_point now)
{
	return Stats::StatVal::makeStatVal(andFilter(
    {
        timeFilter(window,
                   now),
        typeFilter(typeid(CubemapFace)),
		[face](Stats::TimeValueDatum datum)
        {
            return (face == -1) ? true : boost::any_cast<CubemapFace>(datum.value).face == face;
        }
    }),
	[](Stats::TimeValueDatum datum)
    {
        return 0.0;
    },
	boost::accumulators::tag::count(),
    name);
}

// ###### FILTERS ######

boost::function<bool(Stats::TimeValueDatum)> StatsUtils::timeFilter(boost::chrono::microseconds window,
	boost::chrono::steady_clock::time_point now)
{
	return [window, now](Stats::TimeValueDatum datum)
	{
		return (now - datum.time) < window;
	};
}

boost::function<bool(Stats::TimeValueDatum)> StatsUtils::typeFilter(const std::type_info& type)
{
	return [&type](Stats::TimeValueDatum datum)
	{
		return datum.value.type() == type;
	};
}

boost::function<bool(Stats::TimeValueDatum)> StatsUtils::naluFaceFilter(int face)
{
	return [face](Stats::TimeValueDatum datum)
	{
		return (face == -1) ? true : boost::any_cast<NALU>(datum.value).face == face;
	};
}

boost::function<bool(Stats::TimeValueDatum)> StatsUtils::naluStatusFilter(NALU::Status status)
{
	return [status](Stats::TimeValueDatum datum)
	{
		return boost::any_cast<NALU>(datum.value).status == status;
	};
}

boost::function<bool(Stats::TimeValueDatum)> StatsUtils::andFilter(std::initializer_list<boost::function<bool(Stats::TimeValueDatum)> > filtersList)
{
	// Make filters permanent
	std::vector<boost::function<bool(Stats::TimeValueDatum)> > filters(filtersList.size());
	std::copy(filtersList.begin(), filtersList.end(), filters.begin());

	return [filters](Stats::TimeValueDatum datum)
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