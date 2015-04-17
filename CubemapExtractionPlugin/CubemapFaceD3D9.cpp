#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D9.h"

#if SUPPORT_D3D9

CubemapFaceD3D9::CubemapFaceD3D9(
	boost::uint32_t width,
	boost::uint32_t height,
	IDirect3DTexture9* texturePtr,
	IDirect3DSurface9* gpuSurfacePtr,
	IDirect3DSurface9* cpuSurfacePtr,
	D3DFORMAT format,
	D3DLOCKED_RECT lockedRect
	) :
	CubemapFace(width, height),
	texturePtr(texturePtr),
	gpuSurfacePtr(gpuSurfacePtr),
	cpuSurfacePtr(cpuSurfacePtr),
	format(format),
	lockedRect(lockedRect) {

}

CubemapFaceD3D9* CubemapFaceD3D9::create(IDirect3DTexture9* texturePtr) {

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

	return new CubemapFaceD3D9(width, height, texturePtr,
		gpuSurfacePtr, cpuSurfacePtr, textureDescription.Format, lockedRect);
}

void CubemapFaceD3D9::copyFromGPUToCPU() {

	// copy data from GPU to CPU
	HRESULT hr = g_D3D9Device->GetRenderTargetData(this->gpuSurfacePtr, this->cpuSurfacePtr);

	//ZeroMemory(&cubemapFaceD3D9->lockedRect, sizeof(D3DLOCKED_RECT));
	//hr = cubemapFaceD3D9->cpuSurfacePtr->LockRect(&cubemapFaceD3D9->lockedRect, 0, D3DLOCK_READONLY);

	memcpy(this->pixels, this->lockedRect.pBits, this->width * this->height * 4);

	//hr = cubemapFaceD3D9->cpuSurfacePtr->UnlockRect();

}

#endif