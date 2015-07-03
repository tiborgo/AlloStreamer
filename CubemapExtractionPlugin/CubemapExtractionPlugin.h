#pragma once

#include <boost/cstdint.hpp>
#include "UnityPluginInterface.h"
#include "AlloShared/Cubemap.hpp"

// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if SUPPORT_D3D9
    #include <d3d9.h>
    extern IDirect3DDevice9* g_D3D9Device;
#endif

#if SUPPORT_D3D11
    #include <d3d11.h>
    extern ID3D11Device* g_D3D11Device;
#endif

#if SUPPORT_OPENGL
    #if UNITY_WIN || UNITY_LINUX
        #include <GL/gl.h>
    #elif UNITY_OSX
        #include <OpenGL/OpenGL.h>
        #include <OpenGL/gl.h>
    #endif
#endif


#define CUBEMAPEXTRACTIONPLUGIN_ID "HH21V0GKQ98fuU2AHoWidiIRJxIrgDy-CubemapExtractionPlugin"

extern "C" void EXPORT_API ConfigureCubemapFromUnity(void** texturePtrs, int cubemapFacesCount, int resolution);
extern "C" void EXPORT_API ConfigureBinocularsFromUnity(void* texturePtr, int width, int height);
extern "C" void EXPORT_API StopFromUnity();
