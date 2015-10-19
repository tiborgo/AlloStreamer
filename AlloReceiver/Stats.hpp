#pragma once

#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/any.hpp>

#include "AlloShared/StatsUtils.hpp"

namespace AlloReceiver
{
	const int FACE_COUNT = 12;

	Stats::StatValsMaker statValsMaker = [](boost::chrono::microseconds             window,
	                                        boost::chrono::steady_clock::time_point now)
	{
		std::list<Stats::StatVal> statVals;

		statVals.insert(statVals.end(),
		{
			StatsUtils::cubemapsCount("cubemapsCount",
			window,
			now)/*,
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
			now)*/
		});

		for (int face = -1; face < FACE_COUNT; face++)
		{
            std::string faceStr = std::to_string(face);
            
			statVals.insert(statVals.end(),
			{
				StatsUtils::facesCount("facesCount" + faceStr,
                    face,
                    window,
                    now),
                Stats::StatVal::makeStatVal(StatsUtils::andFilter(
                    {
                        StatsUtils::timeFilter(window,
                                               now),
                        StatsUtils::typeFilter(typeid(StatsUtils::Frame)),
                        [face](Stats::TimeValueDatum datum)
                        {
                            return (face == -1) ? true : boost::any_cast<StatsUtils::Frame>(datum.value).face == face;
                        },
                        [face](Stats::TimeValueDatum datum)
                        {
                            return boost::any_cast<StatsUtils::Frame>(datum.value).status == StatsUtils::Frame::RECEIVED;
                        }
                    }),
                    [](Stats::TimeValueDatum datum)
                    {
                        return 0.0;
                    },
                    boost::accumulators::tag::count(),
                    "receivedFrames" + faceStr),
                Stats::StatVal::makeStatVal(StatsUtils::andFilter(
                    {
                        StatsUtils::timeFilter(window,
                                               now),
                        StatsUtils::typeFilter(typeid(StatsUtils::Frame)),
                        [face](Stats::TimeValueDatum datum)
                        {
                          return (face == -1) ? true : boost::any_cast<StatsUtils::Frame>(datum.value).face == face;
                        },
                        [face](Stats::TimeValueDatum datum)
                        {
                            return boost::any_cast<StatsUtils::Frame>(datum.value).status == StatsUtils::Frame::DECODED;
                        }
                    }),
                    [](Stats::TimeValueDatum datum)
                    {
                        return 0.0;
                    },
                    boost::accumulators::tag::count(),
                    "decodedFrames" + faceStr),
                Stats::StatVal::makeStatVal(StatsUtils::andFilter(
                                                                  {
                                                                      StatsUtils::timeFilter(window,
                                                                                             now),
                                                                      StatsUtils::typeFilter(typeid(StatsUtils::Frame)),
                                                                      [face](Stats::TimeValueDatum datum)
                                                                      {
                                                                          return (face == -1) ? true : boost::any_cast<StatsUtils::Frame>(datum.value).face == face;
                                                                      },
                                                                      [face](Stats::TimeValueDatum datum)
                                                                      {
                                                                          return boost::any_cast<StatsUtils::Frame>(datum.value).status == StatsUtils::Frame::COLOR_CONVERTED;
                                                                      }
                                                                  }),
                                            [](Stats::TimeValueDatum datum)
                                            {
                                                return 0.0;
                                            },
                                            boost::accumulators::tag::count(),
                                            "colorConvertedFrames" + faceStr),
                Stats::StatVal::makeStatVal(StatsUtils::andFilter(
                    {
                        [window, now, face](Stats::TimeValueDatum datum)
                        {
                            return (now - datum.time) < window && // time filter
                                datum.value.type() == typeid(StatsUtils::CubemapFace) && // type filter
                                boost::any_cast<StatsUtils::CubemapFace>(datum.value).status == StatsUtils::CubemapFace::ADDED && // status filter
                                (face == -1 || boost::any_cast<StatsUtils::CubemapFace>(datum.value).face == face); // face filter
                        }
                    }),
                    [](Stats::TimeValueDatum datum)
                    {
                        return 0.0;
                    },
                    boost::accumulators::tag::count(),
                    "addedFacesCount" + faceStr),
                Stats::StatVal::makeStatVal(StatsUtils::andFilter(
                    {
                        [window, now, face](Stats::TimeValueDatum datum)
                        {
                          return (now - datum.time) < window && // time filter
                            datum.value.type() == typeid(StatsUtils::CubemapFace) && // type filter
                            boost::any_cast<StatsUtils::CubemapFace>(datum.value).status == StatsUtils::CubemapFace::SCHEDULED && // status filter
                            (face == -1 || boost::any_cast<StatsUtils::CubemapFace>(datum.value).face == face); // face filter
                        }
                    }),
                    [](Stats::TimeValueDatum datum)
                    {
                        return 0.0;
                    },
                    boost::accumulators::tag::count(),
                    "scheduledFacesCount" + faceStr)
				/*StatsUtils::nalusCount("droppedNALUsCount" + std::to_string(face),
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
				now)*/
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

			results["fps"] = results["cubemapsCount"] / seconds;

			//results.insert(
			//{
				

				/*{
					"naluDropRate",
					results["droppedNALUsCount-1"] / results["addedNALUsCount-1"]
				},*/
				/*{
					"fps",
					results["cubemapsCount"] / seconds
				},*/
				/*{
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
				},*/
			//});

			for (int face = -1; face < FACE_COUNT; face++)
			{
				std::string faceStr = std::to_string(face);

				results.insert(
				{
					{
						"facesPS" + faceStr,
						results["facesCount" + faceStr] / seconds
					},
					/*{
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
					},*/
                    {
                        "receivedFrames" + faceStr + "PS",
                        results["receivedFrames" + faceStr] / seconds
                    },
                    {
                        "decodedFrames" + faceStr + "PS",
                        results["decodedFrames" + faceStr] / seconds
                    },
                    {
                        "colorConvertedFrames" + faceStr + "PS",
                        results["colorConvertedFrames" + faceStr] / seconds
                    },
                    {
                        "addedFaces" + faceStr + "PS",
                        results["addedFacesCount" + faceStr] / seconds
                    },
                    {
                        "scheduledFaces" + faceStr + "PS",
                        results["scheduledFacesCount" + faceStr] / seconds
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
		/*stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "NALU drop rate: {naluDropRate:0.1f}" << std::endl;
		stream << "received NALUs/s: {receivedNALUsPS-1:0.1f}; {receivedNALUsBitS:0.1f} MBit/s;" << std::endl;
		for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
		{
			stream << "recvd NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
			for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
			{
				stream << "\t{receivedNALUsPS" << j * 6 + i << ":0.1f}";
			}
			stream << ";" << std::endl;
		}
		stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "processed NALUs/s: {processedNALUsPS-1:0.1f}; {processedNALUsBitS:0.1f} MBit/s;" << std::endl;
		for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
		{
			stream << "prced NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
			for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
			{
				stream << "\t{processedNALUsPS" << j * 6 + i << ":0.1f}";
			}
			stream << ";" << std::endl;
		}

		stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "sent NALUs/s: {sentNALUsPS-1:0.1f}; {sentNALUsBitS:0.1f} MBit/s;" << std::endl;
		for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
		{
			stream << "sent NALUs/s per face (" << ((j == 0) ? "left" : "right") << "):";
			for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
			{
				stream << "\t{sentNALUsPS" << j * 6 + i << ":0.1f}";
			}
			stream << ";" << std::endl;
		}*/
        
        stream << "-------------------------------------------------------------------------------" << std::endl;
        stream << "Received frames/s:" << std::endl;
        for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
        {
            stream << ((j == 0) ? "left" : "right") << ":";
            for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
            {
                stream << "\t{receivedFrames" << j * 6 + i << "PS:0.1f}";
            }
            stream << ";" << std::endl;
        }
        
        stream << "-------------------------------------------------------------------------------" << std::endl;
        stream << "Decoded frames/s:" << std::endl;
        for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
        {
            stream << ((j == 0) ? "left" : "right") << ":";
            for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
            {
                stream << "\t{decodedFrames" << j * 6 + i << "PS:0.1f}";
            }
            stream << ";" << std::endl;
        }
        
        stream << "-------------------------------------------------------------------------------" << std::endl;
        stream << "Color converted frames/s:" << std::endl;
        for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
        {
            stream << ((j == 0) ? "left" : "right") << ":";
            for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
            {
                stream << "\t{colorConvertedFrames" << j * 6 + i << "PS:0.1f}";
            }
            stream << ";" << std::endl;
        }
        
        /*stream << "Decoded:\t";
        for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
        {
            stream << "{receivedFrames" << faceStr << "PS:0.1f}\t";
        }
        stream << ";" << std::endl;*/
        
		stream << "-------------------------------------------------------------------------------" << std::endl;
		stream << "Added faces/s:" << std::endl;
        for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
        {
            stream << ((j == 0) ? "left" : "right") << ":";
            for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
            {
                stream << "\t{addedFaces" << j * 6 + i << "PS:0.1f}";
            }
            stream << ";" << std::endl;
        }
        
        stream << "-------------------------------------------------------------------------------" << std::endl;
        stream << "Scheduled faces/s:" << std::endl;
        for (int j = 0; j < (std::min) (2, FACE_COUNT); j++)
        {
            stream << ((j == 0) ? "left" : "right") << ":";
            for (int i = 0; i < (std::min) (6, FACE_COUNT - j * 6); i++)
            {
                stream << "\t{scheduledFaces" << j * 6 + i << "PS:0.1f}";
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

		return stream.str();
	};
}
