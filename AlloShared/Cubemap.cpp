#include "Cubemap.hpp"

CubemapFace::CubemapFace(Frame* content,
                         int index,
                         Allocator& allocator)
    :
    content(content),
    index(index),
    allocator(allocator)
{
	newFaceFlag = true;
}

CubemapFace::~CubemapFace()
{
    Frame::destroy(content.get());
}
bool CubemapFace::getNewFaceFlag()
{
	return newFaceFlag;
}
void CubemapFace::setNewFaceFlag(bool b)
{
	newFaceFlag = b;
}
int CubemapFace::getIndex()
{
    return index;
}

Frame* CubemapFace::getContent()
{
    return content.get();
}

CubemapFace* CubemapFace::create(Frame* content,
                                 int index,
                                 Allocator& allocator)
{
    void* addr = allocator.allocate(sizeof(CubemapFace));
    return new (addr) CubemapFace(content, index, allocator);
}

void CubemapFace::destroy(CubemapFace* cubemapFace)
{
    cubemapFace->~CubemapFace();
    cubemapFace->allocator.deallocate(cubemapFace, sizeof(CubemapFace));
}

//const size_t Cubemap::MAX_FACES_COUNT = CUBEMAP_MAX_FACES_COUNT;

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

CubemapFace* Cubemap::getFace(int index, bool force)
{
	CubemapFace* temp = faces[index].get();
	if (temp->getNewFaceFlag() || force)
	{
		return temp;
	}
	else
	{
		return NULL;
	}
		
}

/*void Cubemap::resetIsNewFace() {
	for (int i = 0; i < faces.size(); i++) {
		faces[i].get()->isNewFace = true;
	}
}
*/
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

//const size_t StereoCubemap::MAX_EYES_COUNT = STEREOCUBEMAP_MAX_EYES_COUNT;

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