#include "CubemapFace.h"

boost::interprocess::offset_ptr<Cubemap> cubemap;

CubemapFace::CubemapFace(boost::uint32_t width,
                         boost::uint32_t height,
                         int index,
                         AVPixelFormat format,
                         boost::chrono::system_clock::time_point presentationTime,
                         void* pixels)
    :
    width(width), height(height), index(index), format(format),
    presentationTime(presentationTime), pixels(pixels)
{

}

boost::uint32_t CubemapFace::getWidth()
{
    return width;
}

boost::uint32_t CubemapFace::getHeight()
{
    return height;
}

int CubemapFace::getIndex()
{
    return index;
}

AVPixelFormat CubemapFace::getFormat()
{
    return format;
}

boost::chrono::system_clock::time_point CubemapFace::getPresentationTime()
{
    return presentationTime;
}

void* CubemapFace::getPixels()
{
    return pixels.get();
}

boost::interprocess::interprocess_mutex& CubemapFace::getMutex()
{
    return mutex;
}

boost::interprocess::interprocess_condition& CubemapFace::getNewPixelsCondition()
{
    return newPixelsCondition;
}

void CubemapFace::setPresentationTime(boost::chrono::system_clock::time_point presentationTime)
{
    this->presentationTime = presentationTime;
}

Cubemap::Cubemap(std::vector<CubemapFace*>& faces)
{
    for (int i = 0; i < faces.size(); i++)
    {
        this->faces[i] = faces[i];
    }
}

CubemapFace* Cubemap::getFace(int index)
{
    return faces[index].get();
}

int Cubemap::getFacesCount()
{
    int count = 0;

    while (count < MAX_FACES_COUNT && faces[count].get() != nullptr)
    {
        count++;
    }
    return count;
}