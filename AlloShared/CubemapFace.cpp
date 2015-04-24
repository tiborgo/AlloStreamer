#include "CubemapFace.h"

AlloShared_API boost::interprocess::offset_ptr<Cubemap> cubemap;

CubemapFace::CubemapFace(boost::uint32_t width,
	boost::uint32_t height,
	int index,
	AVPixelFormat format,
	PixelAllocator& allocator)
	: width(width), height(height), pixels(allocator.allocate(width * height * 4)),
	index(index), format(format)
{

}

Cubemap::Cubemap(FacePtrAllocator& allocator)
: faces(allocator)
{

}

void Cubemap::setFace(CubemapFace::Ptr& face) {
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
	if (faces.size() <= face->index) {
		faces.resize(face->index + 1);
	}
	faces[face->index] = face;
	newFaceCondition.notify_all();
}

CubemapFace::Ptr Cubemap::getFace(int index) {
	return faces[index];
}

int Cubemap::count() {
	return faces.size();
}