#pragma once

#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D11.h"
#include <boost/thread/mutex.hpp>

#if SUPPORT_D3D11

static AVPixelFormat avPixel2DXGIFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return AV_PIX_FMT_RGBA;
	default:
		return AV_PIX_FMT_NONE;
	}
}

template <typename MemoryAlgorithm>
CubemapFaceD3D11<MemoryAlgorithm>::CubemapFaceD3D11(
	boost::uint32_t width,
	boost::uint32_t height,
	int index,
	PixelAllocator allocator,
	ID3D11Texture2D* gpuTexturePtr,
	ID3D11Texture2D* cpuTexturePtr,
	D3D11_MAPPED_SUBRESOURCE resource,
	D3D11_TEXTURE2D_DESC& description)
	:
	CubemapFace(width,
	height,
	index,
	avPixel2DXGIFormat(description.Format),
	allocator),
	gpuTexturePtr(gpuTexturePtr),
	cpuTexturePtr(cpuTexturePtr),
	resource(resource)
{
}

template <typename MemoryAlgorithm>
CubemapFaceD3D11<MemoryAlgorithm>* CubemapFaceD3D11<MemoryAlgorithm>::create(
	ID3D11Texture2D* texturePtr,
	int index,
	FaceAllocator& faceAllocator,
	PixelAllocator& pixelAllocator)
{

	ID3D11Texture2D* gpuTexturePtr = (ID3D11Texture2D*)texturePtr;

	D3D11_TEXTURE2D_DESC textureDescription;
	gpuTexturePtr->GetDesc(&textureDescription);

	UINT width = textureDescription.Width;
	UINT height = textureDescription.Height;

	textureDescription.BindFlags = 0;
	textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDescription.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D* cpuTexturePtr;
	HRESULT hr = g_D3D11Device->CreateTexture2D(&textureDescription, NULL, &cpuTexturePtr);

	ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
	g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);
	//HRESULT hr2 = g_D3D11Device->CreateDeferredContext(0, &g_D3D11DeviceContext);

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	hr = g_D3D11DeviceContext->Map(cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &resource);
	g_D3D11DeviceContext->Unmap(cpuTexturePtr, subresource);

	CubemapFaceD3D11* addr = faceAllocator.allocate_one().get();

	return new (addr)CubemapFaceD3D11(width,
		height,
		index,
		pixelAllocator,
		gpuTexturePtr,
		cpuTexturePtr,
		resource,
		textureDescription);
}

static boost::mutex d3D11DeviceContextMutex;

template <typename MemoryAlgorithm>
void CubemapFaceD3D11<MemoryAlgorithm>::copyFromGPUToCPU()
{
	presentationTime = boost::chrono::system_clock::now();

	{
		// DirectX 11 is not thread-safe
		boost::mutex::scoped_lock lock(d3D11DeviceContextMutex);

		ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
		g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);

		// copy data from GPU to CPU
		g_D3D11DeviceContext->CopyResource(this->cpuTexturePtr, this->gpuTexturePtr);
	}

	/*ZeroMemory(&this->resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	HRESULT hr = g_D3D11DeviceContext->Map(this->cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &this->resource);*/

	// DirectX 11 is using wrong order of colors in a pixel -> correcting it
	/*char* pixels = (char*)this->pixels.get();
	for (unsigned int i = 0; i < this->width * this->height * 4; i += 4) {
	for (int j = 0; j < 3; j++) {
	pixels[i + j] = ((char*)this->resource.pData)[i + 2 - j];
	}
	}*/
	boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);

	memcpy(this->pixels.get(), this->resource.pData, this->width * this->height * 4);

	//g_D3D11DeviceContext->Unmap(this->cpuTexturePtr, subresource);

	newPixelsCondition.notify_all();
}

#endif