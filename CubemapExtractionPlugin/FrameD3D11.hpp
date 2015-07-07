#pragma once

#include "UnityPluginInterface.h"
#include "AlloShared/Frame.hpp"

#if SUPPORT_D3D11

#include <d3d11.h>

class FrameD3D11 : public Frame
{
public:
	typedef boost::interprocess::offset_ptr<FrameD3D11> Ptr;

	ID3D11Texture2D* const gpuTexturePtr;
	ID3D11Texture2D* const cpuTexturePtr;
	D3D11_MAPPED_SUBRESOURCE resource;

	static FrameD3D11* create(ID3D11Texture2D* texturePtr,
		                      Allocator&       allocator);

protected:
	FrameD3D11(boost::uint32_t width,
               boost::uint32_t height,
			   boost::chrono::system_clock::time_point presentationTime,
               void* pixels,
			   Allocator& allocator,
			   ID3D11Texture2D* gpuTexturePtr,
			   ID3D11Texture2D* cpuTexturePtr,
			   D3D11_MAPPED_SUBRESOURCE resource,
			   D3D11_TEXTURE2D_DESC& description);
};

#endif