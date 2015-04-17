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

class CubemapFaceD3D9 : public CubemapFace {
public:
	IDirect3DTexture9* const texturePtr;
	IDirect3DSurface9* const gpuSurfacePtr;
	IDirect3DSurface9* const cpuSurfacePtr;
	const D3DFORMAT format;
	const D3DLOCKED_RECT lockedRect;

	static CubemapFaceD3D9* create(IDirect3DTexture9* texturePtr);

protected:
	CubemapFaceD3D9(
		boost::uint32_t width,
		boost::uint32_t height,
		IDirect3DTexture9* texturePtr,
		IDirect3DSurface9* gpuSurfacePtr,
		IDirect3DSurface9* cpuSurfacePtr,
		D3DFORMAT format,
		D3DLOCKED_RECT lockedRect
		);
};

#endif

#if SUPPORT_D3D11

#include <d3d11.h>

class CubemapFaceD3D11 : public CubemapFace {
public:
	ID3D11Texture2D* const gpuTexturePtr;
	ID3D11Texture2D* const cpuTexturePtr;
	D3D11_MAPPED_SUBRESOURCE resource;

	static CubemapFaceD3D11* create(ID3D11Texture2D* texturePtr);

protected:

	CubemapFaceD3D11(
		boost::uint32_t width,
		boost::uint32_t height,
		ID3D11Texture2D* gpuTexturePtr,
		ID3D11Texture2D* cpuTexturePtr,
		D3D11_MAPPED_SUBRESOURCE resource
		);
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

