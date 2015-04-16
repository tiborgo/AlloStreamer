#pragma once

#include <boost/cstdint.hpp>

struct CubemapFace {
	boost::uint32_t width;
	boost::uint32_t height;
	void* pixels;
};