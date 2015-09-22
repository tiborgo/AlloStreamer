#include "Frame.hpp"

Frame::Frame(boost::uint32_t                         width,
             boost::uint32_t                         height,
             AVPixelFormat                           format,
             boost::chrono::system_clock::time_point presentationTime,
			 std::string&                            id,
             Allocator&                              allocator)
    :
    allocator(allocator), width(width), height(height), format(format),
	presentationTime(presentationTime), pixels(allocator.allocate(width * height * 4)) // for RGBA
{
	for (int i = 0; i < (std::min)(id.length(), mutexName.size()-1); i++)
	{
		mutexName[i] = id[i];
	}
	for (int i = (std::min)(id.length(), mutexName.size() - 1); i < mutexName.size(); i++)
	{
		mutexName[i] = 0;
	}
}

Frame::~Frame()
{
	allocator.deallocate(pixels.get(), width * height * 4);
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

/*boost::interprocess::interprocess_mutex& Frame::getMutex()
{
    return mutex;
}*/

std::string Frame::getMutexName()
{
	std::string mutexNameStr(mutexName.data());
	return mutexNameStr;
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
					 std::string&                            id,
                     Allocator&                              allocator)
{
    void* addr = allocator.allocate(sizeof(Frame));
    return new (addr) Frame(width, height, format, presentationTime, id, allocator);
}

void Frame::destroy(Frame* frame)
{
    frame->~Frame();
	frame->allocator.deallocate(frame, sizeof(Frame));
}