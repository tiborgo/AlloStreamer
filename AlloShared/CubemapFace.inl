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

template<typename SegmentManager>
Cubemap* Cubemap::create(std::vector<CubemapFace*> faces,
    Allocator<SegmentManager>& allocator)
{
    void* addr = allocator.allocate(sizeof(Cubemap)).get();
    
    return new (addr) Cubemap(faces);
}