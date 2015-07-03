#pragma once

#include "CubemapExtractionPlugin.h"

#if SUPPORT_OPENGL

    struct UserDataOpenGL
    {
        GLuint gpuTextureID;
    };

#endif /* if SUPPORT_OPENGL */