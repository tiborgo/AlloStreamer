#include "CubemapFace.h"

boost::interprocess::offset_ptr<Cubemap> cubemap;

boost::chrono::system_clock::time_point Frame::getPresentationTime()
{
	return presentationTime;
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
    while(count < MAX_FACES_COUNT && faces[count].get() != nullptr)
    {
        count++;
    }
    return count;
}