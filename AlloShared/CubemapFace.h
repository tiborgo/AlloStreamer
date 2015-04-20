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
	const int index;

	virtual void copyFromGPUToCPU() = 0;

protected:
	CubemapFace(boost::uint32_t width, boost::uint32_t height, int index);
};

class AlloShared_API Cubemap {

private:
	std::vector<CubemapFace*> faces;

public:
	void setFace(CubemapFace* face);
	CubemapFace* getFace(int index);
	int count();
};

extern AlloShared_API Cubemap cubemap;