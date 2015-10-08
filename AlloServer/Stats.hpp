#pragma once

#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/any.hpp>

#include "AlloShared/StatsUtils.hpp"

#undef max
#undef min

namespace AlloServer
{
	const int FACE_COUNT = 12;
	const std::vector<int> NALU_TYPES({ -1, 1, 5, 7, 8 });

	Stats::StatValsMaker statValsMaker = [](boost::chrono::microseconds             window,
	                                        boost::chrono::steady_clock::time_point now)
	{
		std::list<Stats::StatVal> statVals;

		for (int face = -1; face < FACE_COUNT; face++)
		{
			statVals.insert(statVals.end(),
			{
				StatsUtils::facesCount("facesCount" + std::to_string(face),
					face,
					window,
					now)/*,
				StatsUtils::nalusCount("sentNALUsCount" + std::to_string(face),
					face,
					StatsUtils::NALU::SENT,
					window,
					now)*/
			});
		}

		for (int naluType : NALU_TYPES)
		{
			statVals.insert(statVals.end(),
			{
			//	Stats::StatVal::makeStatVal(
			//		[naluType](Stats::TimeValueDatum datum)
			//		{
			//			if (datum.value.type() == typeid(StatsUtils::NALU) &&
			//				(naluType == -1 || boost::any_cast<StatsUtils::NALU>(datum.value).type == naluType))
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				return false;
			//			}
			//		},
			//		[](Stats::TimeValueDatum datum)
			//		{
			//			return 0.0;
			//		},
			//		boost::accumulators::tag::count(),
			//		"sent" + std::to_string(naluType) + "NALU"),
			//	Stats::StatVal::makeStatVal(
			//		[naluType](Stats::TimeValueDatum datum)
			//		{
			//			if (datum.value.type() == typeid(StatsUtils::NALU) &&
			//				(naluType == -1 || boost::any_cast<StatsUtils::NALU>(datum.value).type == naluType))
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				return false;
			//			}
			//		},
			//		[](Stats::TimeValueDatum datum)
			//		{
			//			return boost::any_cast<StatsUtils::NALU>(datum.value).size;
			//		},
			//		boost::accumulators::tag::sum(),
			//		"sent" + std::to_string(naluType) + "NALUBit"),
			//	Stats::StatVal::makeStatVal(
			//		[naluType](Stats::TimeValueDatum datum)
			//		{
			//			if (datum.value.type() == typeid(StatsUtils::NALU) &&
			//				(naluType == -1 || boost::any_cast<StatsUtils::NALU>(datum.value).type == naluType))
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				return false;
			//			}
			//		},
			//		[](Stats::TimeValueDatum datum)
			//		{
			//			return boost::any_cast<StatsUtils::NALU>(datum.value).size;
			//		},
			//		boost::accumulators::tag::min(),
			//		"sent" + std::to_string(naluType) + "NALUMin"),
			//	Stats::StatVal::makeStatVal(
			//		[naluType](Stats::TimeValueDatum datum)
			//		{
			//			if (datum.value.type() == typeid(StatsUtils::NALU) &&
			//				(naluType == -1 || boost::any_cast<StatsUtils::NALU>(datum.value).type == naluType))
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				return false;
			//			}
			//		},
			//		[](Stats::TimeValueDatum datum)
			//		{
			//			return boost::any_cast<StatsUtils::NALU>(datum.value).size;
			//		},
			//		boost::accumulators::tag::max(),
			//		"sent" + std::to_string(naluType) + "NALUMax"),
			//	Stats::StatVal::makeStatVal(
			//		[naluType](Stats::TimeValueDatum datum)
			//		{
			//			if (datum.value.type() == typeid(StatsUtils::NALU) &&
			//				(naluType == -1 || boost::any_cast<StatsUtils::NALU>(datum.value).type == naluType))
			//			{
			//				return true;
			//			}
			//			else
			//			{
			//				return false;
			//			}
			//		},
			//		[](Stats::TimeValueDatum datum)
			//		{
			//			return boost::any_cast<StatsUtils::NALU>(datum.value).size;
			//		},
			//		boost::accumulators::tag::mean(),
			//		"sent" + std::to_string(naluType) + "NALUAvg")

				StatsUtils::nalus("sent" + std::to_string(naluType) + "NALU",
					-1,
					StatsUtils::NALU::SENT,
					naluType,
					boost::accumulators::tag::count(),
					window,
					now),
				StatsUtils::nalus("sent" + std::to_string(naluType) + "NALUBit",
					-1,
					StatsUtils::NALU::SENT,
					naluType,
					boost::accumulators::tag::sum(),
					window,
					now),
				StatsUtils::nalus("sent" + std::to_string(naluType) + "NALUMin",
					-1,
					StatsUtils::NALU::SENT,
					naluType,
					boost::accumulators::tag::min(),
					window,
					now),
				StatsUtils::nalus("sent" + std::to_string(naluType) + "NALUMax",
					-1,
					StatsUtils::NALU::SENT,
					naluType,
					boost::accumulators::tag::max(),
					window,
					now),
				StatsUtils::nalus("sent" + std::to_string(naluType) + "NALUAvg",
					-1,
					StatsUtils::NALU::SENT,
					naluType,
					boost::accumulators::tag::mean(),
					window,
					now)
			});
		}

		return statVals;
	};

	Stats::PostProcessorMaker postProcessorMaker = [](boost::chrono::microseconds             window,
		                                              boost::chrono::steady_clock::time_point now)
	{
		Stats::PostProcessor postProcessor = [window, now](std::map<std::string, double>& results)
		{
			unsigned long seconds = boost::chrono::duration_cast<boost::chrono::seconds>(window).count();

			for (int face = -1; face < FACE_COUNT; face++)
			{
				std::string faceStr = std::to_string(face);

				results["facesPS" + faceStr] = results["facesCount" + faceStr] / seconds;

				//results.insert(
				//{
				//	{
				//		"facesPS" + faceStr,
				//		results["facesCount" + faceStr] / seconds
				//	}/*,
				//	{
				//		"sentNALUsPS" + faceStr,
				//		results["sentNALUsCount" + faceStr] / seconds
				//	},*/
				//});
			}

			for (int naluType : NALU_TYPES)
			{
				std::string naluTypeStr = std::to_string(naluType);

				results.insert(
				{
					{
						"sent" + naluTypeStr + "NALUPS",
						results["sent" + naluTypeStr + "NALU"] / seconds
					},
					{
						"sent" + naluTypeStr + "NALUBitPS",
						results["sent" + naluTypeStr + "NALUBit"] * 8.0 / 1000000.0 / seconds
					}
				});
			}
		};

		return postProcessor;
	};

	Stats::FormatStringMaker formatStringMaker = [](boost::chrono::microseconds             window,
		                                            boost::chrono::steady_clock::time_point now)
	{
		std::stringstream stream;

		stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "Sent NALUs (min, max, avg in bytes):" << std::endl;
		stream << "All:      \t{sent-1NALUPS:0.1f}/s; \t{sent-1NALUBitPS:0.1f} MBit/s; \tmin: {sent-1NALUMin:0.0f}; \tmax: {sent-1NALUMax:0.0f}; \tavg: {sent-1NALUAvg:0.1f}" << std::endl;
		stream << "P/B:      \t{sent1NALUPS:0.1f}/s; \t{sent1NALUBitPS:0.1f} MBit/s; \tmin: {sent1NALUMin:0.0f}; \tmax: {sent1NALUMax:0.0f}; \tavg: {sent1NALUAvg:0.1f}" << std::endl;
		stream << "IDR:      \t{sent5NALUPS:0.1f}/s; \t{sent5NALUBitPS:0.1f} MBit/s; \tmin: {sent5NALUMin:0.0f}; \tmax: {sent5NALUMax:0.0f}; \tavg: {sent5NALUAvg:0.1f}" << std::endl;
		stream << "SPS:      \t{sent7NALUPS:0.1f}/s; \t{sent7NALUBitPS:0.1f} MBit/s; \tmin: {sent7NALUMin:0.0f}; \tmax: {sent7NALUMax:0.0f}; \tavg: {sent7NALUAvg:0.1f}" << std::endl;
		stream << "PPS:      \t{sent8NALUPS:0.1f}/s; \t{sent8NALUBitPS:0.1f} MBit/s; \tmin: {sent8NALUMin:0.0f}; \tmax: {sent8NALUMax:0.0f}; \tavg: {sent8NALUAvg:0.1f}" << std::endl;
		/*for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
		{
			stream << ((j == 0) ? "left " : "right") << " 1-6:";
			for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
			{
				stream << "\t{sentNALUsPS" << j * 6 + i << ":0.1f}/s";
			}
			stream << ";" << std::endl;
		}*/

		stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "Encoded cubemap faces:" << std::endl;
		stream << "left 1-6:";
		for (int i = 0; i < 6; i++)
		{
			stream << "\t{facesPS" << i << ":0.1f}/s";
		}
		stream << ";" << std::endl;

		stream << "right 1-6:";
		for (int i = 6; i < 12; i++)
		{
			stream << "\t{facesPS" << i << ":0.1f}/s";
		}
		stream << ";" << std::endl;

		return stream.str();
	};
}
