#pragma once

#include <boost/cstdint.hpp>
#include "UnityPluginInterface.h"
#include "AlloShared/CubemapFace.h"

#if SUPPORT_D3D9

#include <d3d9.h>

class CubemapFaceD3D9 : public CubemapFace {
public:
	typedef boost::interprocess::allocator<CubemapFaceD3D9, CubemapFace::SegmentManager> FaceAllocator;
	typedef boost::interprocess::offset_ptr<CubemapFaceD3D9> Ptr;

	IDirect3DTexture9* const texturePtr;
	IDirect3DSurface9* const gpuSurfacePtr;
	IDirect3DSurface9* const cpuSurfacePtr;
	const D3DFORMAT format;
	const D3DLOCKED_RECT lockedRect;

	static Ptr create(IDirect3DTexture9* texturePtr,
		int index,
		FaceAllocator& faceAllocator,
		PixelAllocator& pixelAllocator);

	void copyFromGPUToCPU();

protected:
	CubemapFaceD3D9(
		boost::uint32_t width,
		boost::uint32_t height,
		int index,
		PixelAllocator& allocator,
		IDirect3DTexture9* texturePtr,
		IDirect3DSurface9* gpuSurfacePtr,
		IDirect3DSurface9* cpuSurfacePtr,
		D3DFORMAT format,
		D3DLOCKED_RECT lockedRect
		);
};

#endif
