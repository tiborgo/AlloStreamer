#include "CubemapFace.h"

AlloShared_API boost::interprocess::offset_ptr<Cubemap> cubemap;

CubemapFace::CubemapFace(boost::uint32_t width,
	boost::uint32_t height,
	int index,
	PixelAllocator& allocator)
	: width(width), height(height), pixels(allocator.allocate(width * height * 4)), index(index)
{

}

Cubemap::Cubemap(FacePtrAllocator& allocator)
: faces(allocator)
{

}

void Cubemap::setFace(CubemapFace::Ptr& face) {
	if (faces.size() <= face->index) {
		faces.resize(face->index + 1);
	}
	faces[face->index] = face;
}

CubemapFace::Ptr Cubemap::getFace(int index) {
	return faces[index];
}

int Cubemap::count() {
	return faces.size();
}