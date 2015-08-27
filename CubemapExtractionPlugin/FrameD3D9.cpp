#pragma once

#include "CubemapExtractionPlugin.h"
#include "FrameD3D9.hpp"

#if SUPPORT_D3D9

CubemapFaceD3D9::CubemapFaceD3D9(boost::uint32_t width,
	                             boost::uint32_t height,
	                             int index,
	                             boost::chrono::system_clock::time_point presentationTime,
	                             void* pixels[MAX_PLANES_COUNT],
	                             Allocator& allocator,
	                             IDirect3DTexture9* texturePtr,
	                             IDirect3DSurface9* gpuSurfacePtr,
	                             IDirect3DSurface9* cpuSurfacePtr,
	                             D3DFORMAT format,
	                             D3DLOCKED_RECT lockedRect)
	:
	Frame(width,
	            height,
				AV_PIX_FMT_NONE,
				presentationTime,
				pixels,
				allocator),
	texturePtr(texturePtr),
	gpuSurfacePtr(gpuSurfacePtr),
	cpuSurfacePtr(cpuSurfacePtr),
	format(format),
	lockedRect(lockedRect) {

}

CubemapFaceD3D9* CubemapFaceD3D9::create(IDirect3DTexture9* texturePtr,
	                                     int index,
	                                     Allocator& allocator)
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

	void* pixels[MAX_PLANES_COUNT];
	for (int i = 0; i < Frame::MAX_PLANES_COUNT; i++)
	{
		pixels[i] = allocator.allocate(width * height * 4);
	}
	void* addr = allocator.allocate(sizeof(CubemapFaceD3D9));
	
	return new (addr) CubemapFaceD3D9(width,
		                              height,
									  index,
									  boost::chrono::system_clock::now(),
									  pixels,
									  allocator,
									  texturePtr,
		                              gpuSurfacePtr,
									  cpuSurfacePtr,
									  textureDescription.Format,
									  lockedRect);
}

#endif