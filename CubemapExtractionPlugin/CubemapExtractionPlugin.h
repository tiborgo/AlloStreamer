#pragma once

#include <boost/cstdint.hpp>
#include "UnityPluginInterface.h"
#include "AlloShared/CubemapFace.h"

#define MULTITHREADED


#ifdef MULTIHREADED
const bool multithreaded = true;
#else
const bool multithreaded = false;
#endif



// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if SUPPORT_D3D9

#include <d3d9.h>

extern IDirect3DDevice9* g_D3D9Device;

#endif

#if SUPPORT_D3D11

#include <d3d11.h>

extern ID3D11Device* g_D3D11Device;

#endif

#if SUPPORT_OPENGL
	#if UNITY_WIN
		#include <gl/GL.h>
	#else
		//		#include <OpenGL/OpenGL.h>
        #include <GL/gl.h>
	#endif
#endif

