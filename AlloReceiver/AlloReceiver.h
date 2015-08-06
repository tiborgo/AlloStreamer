#pragma once

#define ALLORECEIVER_NS au

#if defined (_WIN32) 
	#if defined(AlloReceiver_EXPORTS)
		#define  ALLORECEIVER_API __declspec(dllexport)
	#else
		#define  ALLORECEIVER_API __declspec(dllimport)
	#endif
#else
	#define ALLORECEIVER_API
#endif

#include "CubemapSource.hpp"