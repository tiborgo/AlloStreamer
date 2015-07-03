#include "Cubemap.hpp"

boost::interprocess::offset_ptr<Cubemap> cubemap;

CubemapFace::CubemapFace(boost::uint32_t width,
                         boost::uint32_t height,
                         int index,
                         AVPixelFormat format,
                         boost::chrono::system_clock::time_point presentationTime,
                         void* pixels,
                         void* userData,
                         Allocator& allocator)
    :
    Frame(width,
          height,
          format,
          presentationTime,
          pixels,
          userData,
          allocator),
    index(index)
{

}

CubemapFace::~CubemapFace()
{
}

int CubemapFace::getIndex()
{
    return index;
}

void CubemapFace::setPresentationTime(boost::chrono::system_clock::time_point presentationTime)
{
    this->presentationTime = presentationTime;
}

CubemapFace* CubemapFace::create(boost::uint32_t width,
                                 boost::uint32_t height,
                                 int index,
                                 AVPixelFormat format,
                                 boost::chrono::system_clock::time_point presentationTime,
                                 void* userData,
                                 Allocator& allocator)
{
    void* addr = allocator.allocate(sizeof(CubemapFace));
    void* pixels = allocator.allocate(width * height * 4);
    return new (addr) CubemapFace(width, height, index, format, presentationTime, pixels, userData, allocator);
}

void CubemapFace::destroy(CubemapFace* cubemapFace)
{
    cubemapFace->~CubemapFace();
    cubemapFace->allocator.deallocate(cubemapFace->pixels.get(), cubemapFace->width * cubemapFace->height * 4);
    cubemapFace->allocator.deallocate(cubemapFace, sizeof(CubemapFace));
}

Cubemap::Cubemap(std::vector<CubemapFace*>& faces,
                 Allocator& allocator)
    :
    allocator(allocator)
{
    for (int i = 0; i < faces.size(); i++)
    {
        this->faces[i] = faces[i];
    }
}

Cubemap::~Cubemap()
{
    for (int i = 0; i < getFacesCount(); i++)
    {
        CubemapFace::destroy(faces[i].get());
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

Cubemap* Cubemap::create(std::vector<CubemapFace*> faces,
                         Allocator& allocator)
{
    void* addr = allocator.allocate(sizeof(Cubemap));
    return new (addr) Cubemap(faces, allocator);
}

void Cubemap::destroy(Cubemap* cubemap)
{
    cubemap->~Cubemap();
    cubemap->allocator.deallocate(cubemap, sizeof(Cubemap));
}

StereoCubemap::StereoCubemap(std::vector<Cubemap*>& eyes,
                             Allocator& allocator)
    :
    allocator(allocator)
{
    for (int i = 0; i < eyes.size(); i++)
    {
        this->eyes[i] = eyes[i];
    }
}

StereoCubemap::~StereoCubemap()
{
    for (int i = 0; i < getEyesCount(); i++)
    {
        Cubemap::destroy(eyes[i].get());
    }
}

Cubemap* StereoCubemap::getEye(int index)
{
    return eyes[index].get();
}

int StereoCubemap::getEyesCount()
{
    int count = 0;

    while (count < MAX_EYES_COUNT && eyes[count].get() != nullptr)
    {
        count++;
    }
    return count;
}


StereoCubemap* StereoCubemap::create(std::vector<Cubemap*>& eyes,
                                     Allocator& allocator)
{
    void* addr = allocator.allocate(sizeof(StereoCubemap));
    return new (addr) StereoCubemap(eyes, allocator);
}


void StereoCubemap::destroy(StereoCubemap* stereoCubemap)
{
    stereoCubemap->~StereoCubemap();
    stereoCubemap->allocator.deallocate(stereoCubemap, sizeof(StereoCubemap));
}