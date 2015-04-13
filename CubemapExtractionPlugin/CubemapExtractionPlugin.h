#pragma once

#include <boost/cstdint.hpp>
#include "UnityPluginInterface.h"

#define MULTITHREADED


#ifdef MULTIHREADED
const bool multithreaded = true;
#else
const bool multithreaded = false;
#endif

struct CubemapFace {
	boost::uint32_t width;
	boost::uint32_t height;
	void* pixels;
};

// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if SUPPORT_D3D9

#include <d3d9.h>

struct CubemapFaceD3D9 : public CubemapFace {
	IDirect3DTexture9* texturePtr;
	IDirect3DSurface9* gpuSurfacePtr;
	IDirect3DSurface9* cpuSurfacePtr;
	D3DFORMAT format;
	D3DLOCKED_RECT lockedRect;
};

#endif

#if SUPPORT_D3D11

#include <d3d11.h>

struct CubemapFaceD3D11 : public CubemapFace {
	ID3D11Texture2D* gpuTexturePtr;
	ID3D11Texture2D* cpuTexturePtr;
	D3D11_MAPPED_SUBRESOURCE resource;
};

#endif

#if SUPPORT_OPENGL
#if UNITY_WIN
#include <gl/GL.h>
#else
//		#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif
#endif

extern int cubemapFaceCount;
extern CubemapFace** cubemapFaces;
