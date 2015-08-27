#pragma once

#include "AlloShared/Frame.hpp"
#include "CubemapExtractionPlugin.h"

#if SUPPORT_OPENGL

    class FrameOpenGL : public Frame
    {
    public:
        GLuint getGPUTextureID();
        
        static FrameOpenGL* create(GLuint     gpuTextureID,
                                   Allocator& allocator);
        
    protected:
        FrameOpenGL(boost::uint32_t                         width,
                    boost::uint32_t                         height,
                    AVPixelFormat                           format,
                    boost::chrono::system_clock::time_point presentationTime,
					void*                                   pixels[MAX_PLANES_COUNT],
                    GLuint                                  gpuTextureID,
                    Allocator&                              allocator);
        
        GLuint gpuTextureID;
    };

#endif /* if SUPPORT_OPENGL */