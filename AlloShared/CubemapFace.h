#pragma once

#include <boost/cstdint.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <libavutil/pixfmt.h>
#include <boost/chrono/system_clocks.hpp>

#ifdef _WIN32
	#ifdef AlloShared_EXPORTS
		#define  AlloShared_API __declspec(dllexport)
	#else
		#define  AlloShared_API __declspec(dllimport)
	#endif
#else
	#define AlloShared_API
#endif

// uses 4 byte per pixel
class AlloShared_API CubemapFace {

public:
	typedef boost::interprocess::managed_shared_memory::segment_manager SegmentManager;
	typedef boost::interprocess::allocator<char, SegmentManager> PixelAllocator;
	typedef boost::interprocess::offset_ptr<CubemapFace> Ptr;

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

class AlloShared_API Cubemap {

public:
	typedef boost::interprocess::allocator<CubemapFace::Ptr, CubemapFace::SegmentManager> FacePtrAllocator;
	typedef boost::interprocess::allocator<CubemapFace, CubemapFace::SegmentManager> FaceAllocator;
	typedef boost::interprocess::offset_ptr<Cubemap> Ptr;

	Cubemap(FacePtrAllocator& allocator);

	void setFace(CubemapFace::Ptr& face);
	CubemapFace::Ptr getFace(int index);
	int count();

	boost::interprocess::interprocess_mutex mutex;
	boost::interprocess::interprocess_condition newFaceCondition;

private:
	boost::interprocess::vector<CubemapFace::Ptr, FacePtrAllocator> faces;
};

extern AlloShared_API boost::interprocess::offset_ptr<Cubemap> cubemap;