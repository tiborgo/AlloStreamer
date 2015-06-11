#pragma once

#include "CubemapFaceOpenGL.h"
#include <boost/thread/mutex.hpp>

#if SUPPORT_OPENGL

CubemapFaceOpenGL::CubemapFaceOpenGL(
    boost::uint32_t width,
    boost::uint32_t height,
    int index,
    boost::chrono::system_clock::time_point presentationTime,
    void* pixels,
    GLuint gpuTextureID,
    Allocator& allocator)
	:
	CubemapFace(width,
                height,
                index,
                AV_PIX_FMT_RGB24,
                presentationTime,
                pixels,
                allocator),
	gpuTextureID(gpuTextureID)
{
    std::cout << gpuTextureID << std::endl;
}

template <typename Allocator>
CubemapFaceOpenGL* CubemapFaceOpenGL::create(
     GLuint textureID,
     int index,
     Allocator& allocator)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    void* pixels = allocator.allocate(width * height * 4);
	CubemapFaceOpenGL* addr = (CubemapFaceOpenGL*)allocator.allocate(sizeof(CubemapFaceOpenGL));

	return new (addr) CubemapFaceOpenGL(width,
		height,
		index,
        boost::chrono::system_clock::now(),
		pixels,
		textureID,
        allocator);
}

#endif