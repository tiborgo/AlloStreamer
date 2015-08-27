#include "Frame.hpp"

Frame::Frame(boost::uint32_t                         width,
             boost::uint32_t                         height,
             AVPixelFormat                           format,
             boost::chrono::system_clock::time_point presentationTime,
             void*                                   pixels[MAX_PLANES_COUNT],
             Allocator&                              allocator)
    :
    allocator(allocator), width(width), height(height), format(format),
    presentationTime(presentationTime)
{
	for (int i = 0; i < MAX_PLANES_COUNT; i++)
	{
		this->pixels[i] = pixels[i];
	}
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

void* Frame::getPixels(size_t plane)
{
	assert(plane < 3);
    return pixels[plane].get();
}

boost::interprocess::interprocess_mutex& Frame::getMutex()
{
    return mutex;
}

boost::interprocess::interprocess_condition& Frame::getNewPixelsCondition()
{
    return newPixelsCondition;
}

void Frame::setPresentationTime(boost::chrono::system_clock::time_point presentationTime)
{
    this->presentationTime = presentationTime;
}

Frame* Frame::create(boost::uint32_t                         width,
                     boost::uint32_t                         height,
                     AVPixelFormat                           format,
                     boost::chrono::system_clock::time_point presentationTime,
                     Allocator&                              allocator)
{
    void* addr = allocator.allocate(sizeof(Frame));
	void* pixels[MAX_PLANES_COUNT];
	for (int i = 0; i < Frame::MAX_PLANES_COUNT; i++)
	{
		pixels[i] = allocator.allocate(width * height * 4);
	}
    return new (addr) Frame(width, height, format, presentationTime, pixels, allocator);
}

void Frame::destroy(Frame* frame)
{
    frame->~Frame();
	for (int i = 0; i < Frame::MAX_PLANES_COUNT; i++)
	{
		frame->allocator.deallocate(frame->pixels[i].get(), frame->width * frame->height * 4);
	}
	frame->allocator.deallocate(frame, sizeof(Frame));
}