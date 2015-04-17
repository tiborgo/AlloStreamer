#pragma once

#include <boost/cstdint.hpp>
#include <vector>

#ifdef _WIN32
	#ifdef AlloShared_EXPORTS
		#define  AlloShared_API __declspec(dllexport)
	#else
		#define  AlloShared_API __declspec(dllimport)
	#endif
#else
	#define AlloShared_API
#endif

// uses 4 byte per pixel
class AlloShared_API CubemapFace {
public:
	const boost::uint32_t width;
	const boost::uint32_t height;
	void* const pixels;

protected:
	CubemapFace(boost::uint32_t width, boost::uint32_t height);
};

class AlloShared_API Cubemap {
public:
	std::vector<CubemapFace*> faces;

	void setFace(int faceIndex, CubemapFace* face);
};

extern AlloShared_API Cubemap cubemap;