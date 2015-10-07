#pragma once

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

		for (int face = -1; face < FACE_COUNT; face++)
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

		return statVals;
	};

	Stats::PostProcessorMaker postProcessorMaker = [](boost::chrono::microseconds             window,
		                                              boost::chrono::steady_clock::time_point now)
	{
		Stats::PostProcessor postProcessor = [window, now](std::map<std::string, double>& results)
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

			for (int face = -1; face < FACE_COUNT; face++)
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
		};

		return postProcessor;
	};

	Stats::FormatStringMaker formatStringMaker = [](boost::chrono::microseconds             window,
		                                            boost::chrono::steady_clock::time_point now)
	{
		std::stringstream stream;
		stream << "===============================================================================" << std::endl;
		stream << "Stats for last {duration}:" << std::endl;
		stream << "-------------------------------------------------------------------------------" << std::endl;
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
