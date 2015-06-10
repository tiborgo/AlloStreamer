#pragma once

#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D9.h"

#if SUPPORT_D3D9

template <typename SegmentManager>
CubemapFaceD3D9::CubemapFaceD3D9(
	boost::uint32_t width,
	boost::uint32_t height,
	int index,
	Allocator<SegmentManager>& allocator,
	IDirect3DTexture9* texturePtr,
	IDirect3DSurface9* gpuSurfacePtr,
	IDirect3DSurface9* cpuSurfacePtr,
	D3DFORMAT format,
	D3DLOCKED_RECT lockedRect
	) :
	CubemapFace(width, height, index, AV_PIX_FMT_NONE, allocator),
	texturePtr(texturePtr),
	gpuSurfacePtr(gpuSurfacePtr),
	cpuSurfacePtr(cpuSurfacePtr),
	format(format),
	lockedRect(lockedRect) {

}

template <typename SegmentManager>
CubemapFaceD3D9::Ptr CubemapFaceD3D9::create(
	IDirect3DTexture9* texturePtr,
	int index,
	Allocator<SegmentManager>& allocator)
{

	D3DSURFACE_DESC textureDescription;
	HRESULT hr = texturePtr->GetLevelDesc(0, &textureDescription);

	UINT width = textureDescription.Width;
	UINT height = textureDescription.Height;
	D3DFORMAT format = textureDescription.Format;

	IDirect3DSurface9* gpuSurfacePtr;
	IDirect3DSurface9* cpuSurfacePtr;

	hr = texturePtr->GetSurfaceLevel(0, &gpuSurfacePtr);

	hr = g_D3D9Device->CreateOffscreenPlainSurface(width, height, format,
		D3DPOOL_SYSTEMMEM, &cpuSurfacePtr, NULL);

	D3DLOCKED_RECT lockedRect;
	ZeroMemory(&lockedRect, sizeof(D3DLOCKED_RECT));
	hr = cpuSurfacePtr->LockRect(&lockedRect, 0, D3DLOCK_READONLY);
	hr = cpuSurfacePtr->UnlockRect();

	CubemapFaceD3D9* addr = (CubemapFaceD3D9*)allocator.allocate(sizeof(CubemapFaceD3D9)).get();
	
	return new (addr) CubemapFaceD3D9(width, height, index, allocator, texturePtr,
		gpuSurfacePtr, cpuSurfacePtr, textureDescription.Format, lockedRect);
}

#endif