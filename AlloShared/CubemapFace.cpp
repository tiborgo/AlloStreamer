#include "CubemapFace.h"

AlloShared_API Cubemap cubemap;

CubemapFace::CubemapFace(boost::uint32_t width, boost::uint32_t height)
: width(width), height(height), pixels(new char[width * height * 4]) {

}

void Cubemap::setFace(int faceIndex, CubemapFace* face) {
	if (faces.size() <= faceIndex) {
		faces.resize(faceIndex + 1);
	}
	faces[faceIndex] = face;
}