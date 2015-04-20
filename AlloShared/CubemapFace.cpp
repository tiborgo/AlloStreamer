#include "CubemapFace.h"

AlloShared_API Cubemap cubemap;

CubemapFace::CubemapFace(boost::uint32_t width, boost::uint32_t height, int index)
: width(width), height(height), pixels(new char[width * height * 4]), index(index) {

}

void Cubemap::setFace(CubemapFace* face) {
	if (faces.size() <= face->index) {
		faces.resize(face->index + 1);
	}
	faces[face->index] = face;
}

CubemapFace* Cubemap::getFace(int index) {
	return faces[index];
}

int Cubemap::count() {
	return faces.size();
}