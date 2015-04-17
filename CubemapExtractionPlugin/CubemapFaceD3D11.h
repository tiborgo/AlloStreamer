#pragma once

#include "UnityPluginInterface.h"
#include "AlloShared/CubemapFace.h"

#if SUPPORT_D3D11

#include <d3d11.h>

class CubemapFaceD3D11 : public CubemapFace {
public:
	ID3D11Texture2D* const gpuTexturePtr;
	ID3D11Texture2D* const cpuTexturePtr;
	D3D11_MAPPED_SUBRESOURCE resource;

	static CubemapFaceD3D11* create(ID3D11Texture2D* texturePtr);

	void copyFromGPUToCPU();

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