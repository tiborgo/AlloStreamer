#include "FrameOpenGL.hpp"

#if SUPPORT_OPENGL

FrameOpenGL::FrameOpenGL(boost::uint32_t                         width,
                         boost::uint32_t                         height,
                         AVPixelFormat                           format,
                         boost::chrono::system_clock::time_point presentationTime,
                         void*                                   pixels[MAX_PLANES_COUNT],
                         GLuint                                  gpuTextureID,
                         Allocator&                              allocator)
	:
	Frame(width,
	      height,
		  format,
		  presentationTime,
		  pixels,
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
	void* pixels[MAX_PLANES_COUNT];
	for (int i = 0; i < Frame::MAX_PLANES_COUNT; i++)
	{
		pixels[i] = allocator.allocate(width * height * 4);
	}
    return new (addr) FrameOpenGL(width,
                                  height,
                                  AV_PIX_FMT_RGB24,
                                  boost::chrono::system_clock::time_point(),
                                  pixels,
                                  gpuTextureID,
                                  allocator);
}

#endif