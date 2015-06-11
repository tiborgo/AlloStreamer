#pragma once

#include "UnityPluginInterface.h"
#include "AlloShared/Cubemap.hpp"

#if SUPPORT_D3D11

#include <d3d11.h>

class CubemapFaceD3D11 : public CubemapFace
{
public:
	typedef boost::interprocess::offset_ptr<CubemapFaceD3D11> Ptr;

	ID3D11Texture2D* const gpuTexturePtr;
	ID3D11Texture2D* const cpuTexturePtr;
	D3D11_MAPPED_SUBRESOURCE resource;

	template <typename SegmentManager>
	static CubemapFaceD3D11* create(ID3D11Texture2D*
		texturePtr,
		int face,
		Allocator<SegmentManager>& allocator);

protected:

	template <typename SegmentManager>
	CubemapFaceD3D11(
		boost::uint32_t width,
		boost::uint32_t height,
		int index,
		Allocator<SegmentManager>& allocator,
		ID3D11Texture2D* gpuTexturePtr,
		ID3D11Texture2D* cpuTexturePtr,
		D3D11_MAPPED_SUBRESOURCE resource,
		D3D11_TEXTURE2D_DESC& description
		);
};

#include "CubemapFaceD3D11.inl"

#endif