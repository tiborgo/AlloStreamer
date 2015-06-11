#pragma once

#include <boost/cstdint.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/managed_heap_memory.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <libavutil/pixfmt.h>
#include <boost/chrono/system_clocks.hpp>
#include <array>

template<typename SegmentManager>
using Allocator = boost::interprocess::allocator<boost::uint8_t, SegmentManager>;

class Frame
{
public:
	//template<typename MemoryAlgorithm>
	//using SegmentManager = boost::interprocess::segment_manager_base<MemoryAlgorithm>;
	
	typedef boost::interprocess::offset_ptr<Frame> Ptr;

	const boost::uint32_t width;
	const boost::uint32_t height;
	boost::interprocess::offset_ptr<void> pixels;
	const AVPixelFormat format;
    boost::chrono::system_clock::time_point presentationTime;

	template<typename SegmentManager>
	Frame(boost::uint32_t width,
		boost::uint32_t height,
		AVPixelFormat format,
		Allocator<SegmentManager>& allocator);

	boost::chrono::system_clock::time_point getPresentationTime();
};

class CubemapFace : public Frame
{

public:
	typedef boost::interprocess::offset_ptr<CubemapFace> Ptr;

	const int index;

	template<typename SegmentManager>
	CubemapFace(boost::uint32_t width,
		boost::uint32_t height,
		int index,
		AVPixelFormat format,
		Allocator<SegmentManager>& allocator);

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newPixelsCondition;
};

class Cubemap
{
public:
    typedef boost::interprocess::offset_ptr<Cubemap> Ptr;
    static const int MAX_FACES_COUNT = 6;
    
    CubemapFace* getFace(int index);
    int getFacesCount();
    
    template<typename SegmentManager>
    static Cubemap* create(std::vector<CubemapFace*> faces,
                           Allocator<SegmentManager>& allocator);
    
protected:
    Cubemap(std::vector<CubemapFace*>& faces);
    
private:
    std::array<CubemapFace::Ptr, MAX_FACES_COUNT> faces;
    boost::interprocess::interprocess_mutex mutex;
};

typedef boost::interprocess::managed_heap_memory::segment_manager HeapSegmentManager;
// Create the cubemap in shared memory
typedef boost::interprocess::managed_shared_memory::segment_manager ShmSegmentManager;

extern boost::interprocess::offset_ptr<Cubemap> cubemap;

#include "CubemapFace.inl"