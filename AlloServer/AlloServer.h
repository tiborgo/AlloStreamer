#pragma once

#ifdef _WIN32
	#ifdef AlloServer_EXPORTS
		#define  AlloServer_API __declspec(dllexport)
	#else
		#define  AlloServer_API __declspec(dllimport)
	#endif
#else
	#define AlloServer_API
#endif

void AlloServer_API startRTSP();