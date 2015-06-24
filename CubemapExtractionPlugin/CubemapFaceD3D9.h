#pragma once

#include <boost/cstdint.hpp>
#include "UnityPluginInterface.h"
#include "AlloShared/Cubemap.hpp"

#if SUPPORT_D3D9

#include <d3d9.h>

class CubemapFaceD3D9 : public CubemapFace
{
public:
	typedef boost::interprocess::offset_ptr<CubemapFaceD3D9> Ptr;

	IDirect3DTexture9* const texturePtr;
	IDirect3DSurface9* const gpuSurfacePtr;
	IDirect3DSurface9* const cpuSurfacePtr;
	const D3DFORMAT format;
	const D3DLOCKED_RECT lockedRect;

	static CubemapFaceD3D9* create(IDirect3DTexture9* texturePtr,
		                           int index,
		                           Allocator& allocator);

protected:
	CubemapFaceD3D9(boost::uint32_t width,
		            boost::uint32_t height,
		            int index,
		            boost::chrono::system_clock::time_point presentationTime,
		            void* pixels,
		            Allocator& allocator,
		            IDirect3DTexture9* texturePtr,
		            IDirect3DSurface9* gpuSurfacePtr,
		            IDirect3DSurface9* cpuSurfacePtr,
		            D3DFORMAT format,
		            D3DLOCKED_RECT lockedRect);
};

#endif
