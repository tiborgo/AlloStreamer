#include "FrameOpenGL.hpp"

#if SUPPORT_OPENGL

FrameOpenGL::FrameOpenGL(boost::uint32_t                         width,
                         boost::uint32_t                         height,
                         AVPixelFormat                           format,
                         boost::chrono::system_clock::time_point presentationTime,
                         GLuint                                  gpuTextureID,
                         Allocator&                              allocator)
	:
	Frame(width,
	      height,
		  format,
		  presentationTime,
		  allocator),
	gpuTextureID(gpuTextureID)
{
}

GLuint FrameOpenGL::getGPUTextureID()
{
    return gpuTextureID;
}

FrameOpenGL* FrameOpenGL::create(GLuint     gpuTextureID,
                                 Allocator& allocator)
{
    glBindTexture(GL_TEXTURE_2D, gpuTextureID);
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    
    void* addr = allocator.allocate(sizeof(FrameOpenGL));
    return new (addr) FrameOpenGL(width,
                                  height,
                                  AV_PIX_FMT_RGB24,
                                  boost::chrono::system_clock::time_point(),
                                  gpuTextureID,
                                  allocator);
}

#endif