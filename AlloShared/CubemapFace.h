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

	virtual void copyFromGPUToCPU() = 0;

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newPixelsCondition;
};

template<typename SegmentManager>
class Cubemap
{
public:
	/*typedef boost::interprocess::allocator<typename Face, typename Face::SegmentManager>
		FaceAllocator;*/
	typedef boost::interprocess::offset_ptr<Cubemap> Ptr;

	Cubemap(Allocator<SegmentManager>& allocator);

	void setFace(CubemapFace::Ptr& face);
	typename CubemapFace::Ptr getFace(int index);
	int count();

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newFaceCondition;

private:
	typedef boost::interprocess::allocator<CubemapFace::Ptr, SegmentManager>
		FacePtrAllocator;

	boost::interprocess::vector<CubemapFace::Ptr, FacePtrAllocator> faces;
};

typedef boost::interprocess::managed_heap_memory::segment_manager HeapSegmentManager;
// Create the cubemap in shared memory
typedef boost::interprocess::managed_shared_memory::segment_manager ShmSegmentManager;
typedef Cubemap<ShmSegmentManager> CubemapImpl;
extern boost::interprocess::offset_ptr<CubemapImpl> cubemap;

#include "CubemapFace.inl"