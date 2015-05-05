#pragma once

template <typename SegmentManager>
Frame::Frame(boost::uint32_t width,
	boost::uint32_t height,
	AVPixelFormat format,
	Allocator<SegmentManager>& allocator)
	: width(width),
	height(height),
	pixels(allocator.allocate(width * height * 4)),
	format(format)
{

}

template <typename SegmentManager>
CubemapFace::CubemapFace(boost::uint32_t width,
	boost::uint32_t height,
	int index,
	AVPixelFormat format,
	Allocator<SegmentManager>& allocator)
	: Frame(width, height, format, allocator),
	index(index)
{

}

template <typename SegmentManager>
Cubemap<SegmentManager>::Cubemap(Allocator<SegmentManager>& allocator)
: faces(FacePtrAllocator(allocator.get_segment_manager()))
{

}

template <typename SegmentManager>
void Cubemap<SegmentManager>::setFace(CubemapFace::Ptr& face)
{
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
	if (faces.size() <= face->index) {
		faces.resize(face->index + 1);
	}
	faces[face->index] = face;
	newFaceCondition.notify_all();
}

template <typename SegmentManager>
CubemapFace::Ptr Cubemap<SegmentManager>::getFace(int index)
{
	return faces[index];
}

template <typename SegmentManager>
int Cubemap<SegmentManager>::count()
{
	return faces.size();
}