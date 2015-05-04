#pragma once

#include "CubemapFace.h"

template <typename MemoryAlgorithm>
CubemapFace<MemoryAlgorithm>::CubemapFace(boost::uint32_t width,
	boost::uint32_t height,
	int index,
	AVPixelFormat format,
	PixelAllocator& allocator)
	: width(width), height(height), pixels(allocator.allocate(width * height * 4)),
	index(index), format(format)
{

}

template <typename MemoryAlgorithm>
boost::chrono::system_clock::time_point CubemapFace<MemoryAlgorithm>::getPresentationTime()
{
	return presentationTime;
}

template <typename MemoryAlgorithm>
Cubemap<MemoryAlgorithm>::Cubemap(FacePtrAllocator& allocator)
: faces(allocator)
{

}

template <typename MemoryAlgorithm>
void Cubemap<MemoryAlgorithm>::setFace(typename CubemapFace<MemoryAlgorithm>::Ptr& face)
{
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
	if (faces.size() <= face->index) {
		faces.resize(face->index + 1);
	}
	faces[face->index] = face;
	newFaceCondition.notify_all();
}

template <typename MemoryAlgorithm>
typename CubemapFace<MemoryAlgorithm>::Ptr Cubemap<MemoryAlgorithm>::getFace(int index)
{
	return faces[index];
}

template <typename MemoryAlgorithm>
int Cubemap<MemoryAlgorithm>::count()
{
	return faces.size();
}