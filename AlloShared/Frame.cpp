#include "Frame.hpp"

Frame::Frame(boost::uint32_t                         width,
             boost::uint32_t                         height,
             AVPixelFormat                           format,
             boost::chrono::system_clock::time_point presentationTime,
             void*                                   pixels,
             void*                                   userData,
             Allocator&                              allocator)
    :
    allocator(allocator), width(width), height(height), format(format),
    presentationTime(presentationTime), pixels(pixels), userData(userData)
{

}

Frame::~Frame()
{
}

boost::uint32_t Frame::getWidth()
{
    return width;
}

boost::uint32_t Frame::getHeight()
{
    return height;
}

AVPixelFormat Frame::getFormat()
{
    return format;
}

boost::chrono::system_clock::time_point Frame::getPresentationTime()
{
    return presentationTime;
}

void* Frame::getPixels()
{
    return pixels.get();
}

boost::interprocess::interprocess_mutex& Frame::getMutex()
{
    return mutex;
}

boost::interprocess::interprocess_condition& Frame::getNewPixelsCondition()
{
    return newPixelsCondition;
}

void* Frame::getUserData()
{
    return userData.get();
}

void Frame::setPresentationTime(boost::chrono::system_clock::time_point presentationTime)
{
    this->presentationTime = presentationTime;
}

Frame* Frame::create(boost::uint32_t                         width,
                     boost::uint32_t                         height,
                     AVPixelFormat                           format,
                     boost::chrono::system_clock::time_point presentationTime,
                     void*                                   userData,
                     Allocator&                              allocator)
{
    void* addr = allocator.allocate(sizeof(Frame));
    void* pixels = allocator.allocate(width * height * 4);
    return new (addr) Frame(width, height, format, presentationTime, pixels, userData, allocator);
}

void Frame::destroy(Frame* Frame)
{
    Frame->~Frame();
    Frame->allocator.deallocate(Frame->pixels.get(), Frame->width * Frame->height * 4);
    Frame->allocator.deallocate(Frame, sizeof(Frame));
}