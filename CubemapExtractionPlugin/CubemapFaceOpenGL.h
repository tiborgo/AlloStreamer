#pragma once

#include "CubemapExtractionPlugin.h"
#include "AlloShared/CubemapFace.h"

#if SUPPORT_OPENGL

    class CubemapFaceOpenGL : public CubemapFace
    {
        public:
            typedef boost::interprocess::offset_ptr<CubemapFaceOpenGL> Ptr;

            const GLuint gpuTextureID;

            template <typename Allocator>
            static CubemapFaceOpenGL* create(GLuint textureID,
                                             int face,
                                             Allocator& allocator);

        protected:
            CubemapFaceOpenGL(boost::uint32_t width,
                              boost::uint32_t height,
                              int index,
                              boost::chrono::system_clock::time_point presentationTime,
                              void* pixels,
                              GLuint gpuTextureID,
                              Allocator& allocator);
    };

    #include "CubemapFaceOpenGL.inl"

#endif /* if SUPPORT_OPENGL */