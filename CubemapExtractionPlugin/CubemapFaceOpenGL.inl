#pragma once

#include "CubemapFaceOpenGL.h"
#include <boost/thread/mutex.hpp>

#if SUPPORT_OPENGL

template <typename SegmentManager>
CubemapFaceOpenGL::CubemapFaceOpenGL(
    boost::uint32_t width,
    boost::uint32_t height,
    int index,
    Allocator<SegmentManager>& allocator,
    GLuint gpuTextureID)
	:
	CubemapFace(width,
	height,
	index,
	AV_PIX_FMT_RGB24,
	allocator),
	gpuTextureID(gpuTextureID)
{
    std::cout << gpuTextureID << std::endl;
}

template <typename SegmentManager>
CubemapFaceOpenGL* CubemapFaceOpenGL::create(
     GLuint textureID,
     int index,
     Allocator<SegmentManager>& allocator)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	CubemapFaceOpenGL* addr = (CubemapFaceOpenGL*)allocator.allocate(sizeof(CubemapFaceOpenGL)).get();

	return new (addr)CubemapFaceOpenGL(width,
		height,
		index,
		allocator,
		textureID);
}

#endif