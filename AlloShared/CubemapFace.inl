#pragma once

template<typename SegmentManager>
CubemapFace* CubemapFace::create(boost::uint32_t width,
    boost::uint32_t height,
    int index,
    AVPixelFormat format,
    boost::chrono::system_clock::time_point presentationTime,
    Allocator<SegmentManager>& allocator)
{
    void* addr = allocator.allocate(sizeof(Cubemap)).get();
    void* pixels = allocator.allocate(width * height * 4).get();
    return new (addr) CubemapFace(width, height, index, format, presentationTime, pixels);
}

template<typename SegmentManager>
Cubemap* Cubemap::create(std::vector<CubemapFace*> faces,
    Allocator<SegmentManager>& allocator)
{
    void* addr = allocator.allocate(sizeof(Cubemap)).get();
    return new (addr) Cubemap(faces);
}