#include "CubemapExtractionPlugin.h"
#include "FrameD3D11.hpp"
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

FrameD3D11::FrameD3D11(boost::uint32_t width,
                                   boost::uint32_t height,
								   boost::chrono::system_clock::time_point presentationTime,
                                   void* pixels,
	                               Allocator& allocator,
	                               ID3D11Texture2D* gpuTexturePtr,
	                               ID3D11Texture2D* cpuTexturePtr,
	                               D3D11_MAPPED_SUBRESOURCE resource,
	                               D3D11_TEXTURE2D_DESC& description)
	:
	Frame(width,
	      height,
          avPixel2DXGIFormat(description.Format),
	      presentationTime,
		  pixels,
	      allocator),
	gpuTexturePtr(gpuTexturePtr),
	cpuTexturePtr(cpuTexturePtr),
	resource(resource)
{
}

FrameD3D11* FrameD3D11::create(ID3D11Texture2D* texturePtr,
	                                       Allocator& allocator)
{

	ID3D11Texture2D* gpuTexturePtr = (ID3D11Texture2D*)texturePtr;

	D3D11_TEXTURE2D_DESC textureDescription;
	gpuTexturePtr->GetDesc(&textureDescription);

	UINT width = textureDescription.Width;
	UINT height = textureDescription.Height;

	textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//textureDescription.Usage = D3D11_USAGE_STAGING;

	if (textureDescription.Format != DXGI_FORMAT_R8G8B8A8_TYPELESS)
	{
		std::cerr << "Can only handle render textures with format DXGI_FORMAT_R8G8B8A8_TYPELESS" << std::endl;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = textureDescription.Width;
	desc.Height = textureDescription.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* cpuTexturePtr;
	//HRESULT hr = g_D3D11Device->CreateTexture2D(&textureDescription, NULL, &cpuTexturePtr);

	g_D3D11Device->CreateTexture2D(&desc, NULL, &cpuTexturePtr);

	//ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
	//g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);
	//HRESULT hr2 = g_D3D11Device->CreateDeferredContext(0, &g_D3D11DeviceContext);

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	/*hr = g_D3D11DeviceContext->Map(cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &resource);
	g_D3D11DeviceContext->Unmap(cpuTexturePtr, subresource);*/

	void* pixels = allocator.allocate(width * height * 4);
	void* addr = allocator.allocate(sizeof(FrameD3D11));

	return new (addr) FrameD3D11(width,
		                         height,
							     boost::chrono::system_clock::now(),
							     pixels,
						         allocator,
		                         gpuTexturePtr,
		                         cpuTexturePtr,
		                         resource,
		                         textureDescription);
}

#endif