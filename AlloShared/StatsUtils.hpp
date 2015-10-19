#pragma once

#include "Stats.hpp"

class StatsUtils
{
public:
	// PARAMETERS
    class NALU
    {
    public:
        enum Status {RECEIVED, DROPPED, ADDED, PROCESSED, SENT};
        
        NALU(int type, size_t size, int face, Status status) : type(type), size(size), face(face), status(status) {}
        int    type;
        size_t size;
        int    face;
        Status status;
    };
    
    class Frame
    {
    public:
        enum Status {RECEIVED, DECODED, COLOR_CONVERTED};
        
        Frame(int type, size_t size, int face, Status status) : type(type), size(size), face(face), status(status) {}
        int    type;
        size_t size;
        int    face;
        Status status;
    };
    
    class CubemapFace
    {
    public:
        enum Status {ADDED, DISPLAYED, SCHEDULED};
        
        CubemapFace(int face, Status status) : face(face), status(status) {}
        int face;
        Status status;
    };

    class Cubemap
    {
    };
    
    // STAT VALS
	static Stats::StatVal nalusBitSum  (const std::string&                      name,
                                        int                                     face,
                                        NALU::Status                            status,
                                        boost::chrono::microseconds             window,
                                        boost::chrono::steady_clock::time_point now);
	static Stats::StatVal nalusCount   (const std::string&                      name,
                                        int                                     face,
                                        NALU::Status                            status,
                                        boost::chrono::microseconds             window,
                                        boost::chrono::steady_clock::time_point now);
	static Stats::StatVal cubemapsCount(const std::string&                      name,
                                        boost::chrono::microseconds             window,
                                        boost::chrono::steady_clock::time_point now);
	static Stats::StatVal facesCount   (const std::string&                      name,
                                        int                                     face,
                                        boost::chrono::microseconds             window,
                                        boost::chrono::steady_clock::time_point now);

	// FILTERS
	static boost::function<bool(Stats::TimeValueDatum)> timeFilter      (boost::chrono::microseconds window,
		                                                                 boost::chrono::steady_clock::time_point now);
	static boost::function<bool(Stats::TimeValueDatum)> typeFilter      (const std::type_info& type);
	static boost::function<bool(Stats::TimeValueDatum)> naluFaceFilter  (int face);
	static boost::function<bool(Stats::TimeValueDatum)> naluStatusFilter(NALU::Status status);
	static boost::function<bool(Stats::TimeValueDatum)> andFilter       (std::initializer_list<boost::function<bool(Stats::TimeValueDatum)> > filters);
};

