#pragma once

template<typename Allocator>
CubemapFace* CubemapFace::create(boost::uint32_t width,
    boost::uint32_t height,
    int index,
    AVPixelFormat format,
    boost::chrono::system_clock::time_point presentationTime,
    Allocator& allocator)
{
    boost::interprocess::offset_ptr<void> addr(allocator.allocate(sizeof(CubemapFace)));
    boost::interprocess::offset_ptr<void> pixels(allocator.allocate(width * height * 4));
    return new (addr.get()) CubemapFace(width, height, index, format, presentationTime, pixels.get());
}

template<typename Allocator>
Cubemap* Cubemap::create(std::vector<CubemapFace*> faces,
    Allocator& allocator)
{
    boost::interprocess::offset_ptr<void> addr(allocator.allocate(sizeof(Cubemap)));
    return new (addr.get()) Cubemap(faces);
}
