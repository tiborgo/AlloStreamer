#pragma once

#include <boost/cstdint.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <libavutil/pixfmt.h>
#include <boost/chrono/system_clocks.hpp>

#include "API.h"

template<typename MemoryAlgorithm>
class CubemapFace {

public:
	typedef boost::interprocess::segment_manager_base<MemoryAlgorithm> SegmentManager;
	//typedef boost::interprocess::managed_shared_memory::segment_manager SegmentManager;
	typedef boost::interprocess::allocator<char, SegmentManager> PixelAllocator;
	typedef boost::interprocess::offset_ptr<CubemapFace<MemoryAlgorithm>> Ptr;

	const boost::uint32_t width;
	const boost::uint32_t height;
	boost::interprocess::offset_ptr<void> pixels;
	const int index;
	const AVPixelFormat format;

	CubemapFace(boost::uint32_t width,
		boost::uint32_t height,
		int index,
		AVPixelFormat format,
		PixelAllocator& allocator);

	virtual void copyFromGPUToCPU() = 0;

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newPixelsCondition;

	boost::chrono::system_clock::time_point getPresentationTime();

protected:
	boost::chrono::system_clock::time_point presentationTime;
};

template<typename MemoryAlgorithm>
class Cubemap {

public:
	typedef CubemapFace<MemoryAlgorithm> Face;
	typedef boost::interprocess::allocator<typename Face::Ptr, typename Face::SegmentManager>
		FacePtrAllocator;
	typedef boost::interprocess::allocator<typename Face, typename Face::SegmentManager>
		FaceAllocator;
	typedef boost::interprocess::offset_ptr<Cubemap> Ptr;

	Cubemap(FacePtrAllocator& allocator);

	void setFace(typename Face::Ptr& face);
	typename Face::Ptr getFace(int index);
	int count();

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newFaceCondition;

private:
	boost::interprocess::vector<typename Face::Ptr, FacePtrAllocator> faces;
};

// Create the cubemap in shared memory
typedef boost::interprocess::managed_shared_memory::segment_manager::memory_algorithm
	ShmMemoryAlgorithm;
typedef Cubemap<ShmMemoryAlgorithm> CubemapImpl;
extern AlloShared_API boost::interprocess::offset_ptr<CubemapImpl> cubemap;

#include "CubemapFace.inl"