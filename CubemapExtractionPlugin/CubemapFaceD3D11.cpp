#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D11.h"

#if SUPPORT_D3D11

CubemapFaceD3D11::CubemapFaceD3D11(
	boost::uint32_t width,
	boost::uint32_t height,
	int index,
	ID3D11Texture2D* gpuTexturePtr,
	ID3D11Texture2D* cpuTexturePtr,
	D3D11_MAPPED_SUBRESOURCE resource
	) :
	CubemapFace(width, height, index),
	gpuTexturePtr(gpuTexturePtr),
	cpuTexturePtr(cpuTexturePtr),
	resource(resource) {
}

CubemapFaceD3D11* CubemapFaceD3D11::create(ID3D11Texture2D* texturePtr, int face) {

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

	return new CubemapFaceD3D11(width, height, face, gpuTexturePtr, cpuTexturePtr, resource);
}

void CubemapFaceD3D11::copyFromGPUToCPU() {

	ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
	g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);

	// copy data from GPU to CPU
	g_D3D11DeviceContext->CopyResource(this->cpuTexturePtr, this->gpuTexturePtr);
		
	//ZeroMemory(&cubemapFaceD3D11->resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	//HRESULT hr = g_D3D11DeviceContext->Map(cubemapFaceD3D11->cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &cubemapFaceD3D11->resource);
		
	// DirectX 11 is using wrong order of colors in a pixel -> correcting it
	for (unsigned int i = 0; i < this->width * this->height * 4; i += 4) {
		for (int j = 0; j < 3; j++) {
			((char*)this->pixels)[i + j] = ((char*)this->resource.pData)[i + 2 - j];
		}
	}
		
	//g_D3D11DeviceContext->Unmap(cubemapFaceD3D11->cpuTexturePtr, subresource);
}

#endif