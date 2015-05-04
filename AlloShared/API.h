#pragma once

#ifdef _WIN32
	#ifdef AlloShared_EXPORTS
		#define  AlloShared_API __declspec(dllexport)
	#else
		#define  AlloShared_API __declspec(dllimport)
	#endif
#else
	#define AlloShared_API
#endif
