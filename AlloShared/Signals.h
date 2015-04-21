#pragma once

#include <boost/signal.hpp>
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



extern AlloShared_API boost::signal<void(int)> extractedCubemapFace;