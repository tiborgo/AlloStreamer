// Example low level rendering Unity plugin

#include <math.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>

#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D9.h"
#include "CubemapFaceD3D11.h"
#include "UserDataOpenGL.hpp"
#include "AlloShared/config.h"
#include "AlloShared/Process.h"
#include "AlloServer/AlloServer.h"

CubemapFace* getCubemapFaceFromTexture(void* texturePtr, int index);

// --------------------------------------------------------------------------
// Helper utilities

static ShmAllocator* shmAllocator = nullptr;
static Process* thisProcess = nullptr;
static Process alloServerProcess(ALLOSERVER_ID, false);
static boost::chrono::system_clock::time_point presentationTime;
static boost::mutex d3D11DeviceContextMutex;
static boost::interprocess::managed_shared_memory shm;

struct CubemapConfig
{
    void** texturePtrs;
    int facesCount;
    int resolution;
};
static CubemapConfig* cubemapConfig = nullptr;

struct BinocularsConfig
{
    void* texturePtr;
    int width;
    int height;
};
static BinocularsConfig* binocularsConfig = nullptr;

GLuint tex;

// Prints a string
static void DebugLog (const char* str)
{
	#if UNITY_WIN
	OutputDebugStringA (str);
	#else
	printf ("%s", str);
	#endif
}

void allocateCubemap(CubemapConfig* cubemapConfig)
{
    shm.destroy<Cubemap::Ptr>("Cubemap");
    
    if (cubemapConfig)
    {
        std::vector<CubemapFace*> faces;
        for (int i = 0; i < cubemapConfig->facesCount; i++)
        {
            faces.push_back(getCubemapFaceFromTexture(cubemapConfig->texturePtrs[i], i));
        }
        
        cubemap = Cubemap::create(faces, *shmAllocator);
        Cubemap::Ptr cubemapPtr = *shm.construct<Cubemap::Ptr>("Cubemap")(cubemap.get());
    }
    else
    {
        cubemap = nullptr;
    }
}

void allocateBinoculars(BinocularsConfig* binocularsConfig)
{
}

void allocateSHM(CubemapConfig* cubemapConfig, BinocularsConfig* binocularsConfig)
{
    unsigned long shmSize = 65536;
    if (cubemapConfig)
    {
        shmSize += cubemapConfig->resolution * cubemapConfig->resolution * 4 * cubemapConfig->facesCount +
                   sizeof(Cubemap) + cubemapConfig->facesCount * sizeof(CubemapFace);
    }
    if (binocularsConfig)
    {
        shmSize += binocularsConfig->width * binocularsConfig->height * 4 + sizeof(Frame);
    }
    
    boost::interprocess::shared_memory_object::remove(SHM_NAME);
    
    shm = boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create,
                                                     SHM_NAME,
                                                     shmSize);
    
    shmAllocator = new ShmAllocator(*(new ShmAllocator::BoostShmAllocator(shm.get_segment_manager())));
    
    allocateCubemap(cubemapConfig);
    allocateBinoculars(binocularsConfig);
    
    thisProcess = new Process(CUBEMAPEXTRACTIONPLUGIN_ID, true);
}



void releaseSHM()
{
    shm.destroy<Cubemap::Ptr>("Cubemap");
    cubemap = nullptr;
    boost::interprocess::shared_memory_object::remove(SHM_NAME);
    delete thisProcess;
    thisProcess = nullptr;
}

// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;


// Actual setup/teardown functions defined below
#if SUPPORT_D3D9
IDirect3DDevice9* g_D3D9Device;
#endif
#if SUPPORT_D3D11
ID3D11Device* g_D3D11Device;
#endif


extern "C" void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
	// Set device type to -1, i.e. "not recognized by our plugin"
	g_DeviceType = -1;
	
	#if SUPPORT_D3D9
	// D3D9 device, remember device pointer and device type.
	// The pointer we get is IDirect3DDevice9.
	if (deviceType == kGfxRendererD3D9)
	{
		DebugLog ("Set D3D9 graphics device\n");
		g_DeviceType = deviceType;
		g_D3D9Device = (IDirect3DDevice9*)device;
	}
	#endif

	#if SUPPORT_D3D11
	// D3D11 device, remember device pointer and device type.
	// The pointer we get is ID3D11Device.
	if (deviceType == kGfxRendererD3D11)
	{
		DebugLog ("Set D3D11 graphics device\n");
		g_DeviceType = deviceType;
		g_D3D11Device = (ID3D11Device*)device;
	}
	#endif

	#if SUPPORT_OPENGL
	// If we've got an OpenGL device, remember device type. There's no OpenGL
	// "device pointer" to remember since OpenGL always operates on a currently set
	// global context.
	if (deviceType == kGfxRendererOpenGL)
	{
		DebugLog ("Set OpenGL graphics device\n");
		g_DeviceType = deviceType;
	}
	#endif
}

CubemapFace* getCubemapFaceFromTexture(void* texturePtr, int index)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
    
    Frame* content = nullptr;

#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
        face = CubemapFaceD3D9::create(
			(IDirect3DTexture9*)texturePtr,
			index,
			*shmAllocator);
	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		face = CubemapFaceD3D11::create(
			(ID3D11Texture2D*)texturePtr,
			index,
			*shmAllocator);
	}
#endif


#if SUPPORT_OPENGL
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
        GLuint textureID = (GLuint)(size_t)texturePtr;
        glBindTexture(GL_TEXTURE_2D, textureID);
        int width, height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        
        UserDataOpenGL* userDataOpenGL = new UserDataOpenGL;
        userDataOpenGL->gpuTextureID = textureID;
        
        content = Frame::create(width,
                                height,
                                AV_PIX_FMT_RGB24,
                                presentationTime,
                                userDataOpenGL,
                                *shmAllocator);
	}
#endif
    
    return CubemapFace::create(content,
                               index,
                               *shmAllocator);
}

void copyFromGPUToCPU(Frame* frame)
{
    frame->setPresentationTime(presentationTime);
    
    // PREPARE COPYING
    
#if SUPPORT_D3D9
    // D3D9 case
    if (g_DeviceType == kGfxRendererD3D9)
    {
        CubemapFaceD3D9* faceD3D9 = (CubemapFaceD3D9*)face;
        // copy data from GPU to CPU
        HRESULT hr = g_D3D9Device->GetRenderTargetData(faceD3D9->gpuSurfacePtr, faceD3D9->cpuSurfacePtr);
    }
#endif
    
    
#if SUPPORT_D3D11
    // D3D11 case
    if (g_DeviceType == kGfxRendererD3D11)
    {
        CubemapFaceD3D11* faceD3D11 = (CubemapFaceD3D11*)face;
        
        // DirectX 11 is not thread-safe
        boost::mutex::scoped_lock lock(d3D11DeviceContextMutex);
        
        ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
        g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);
        
        // copy data from GPU to CPU
        g_D3D11DeviceContext->CopyResource(faceD3D11->cpuTexturePtr, faceD3D11->gpuTexturePtr);
    }
#endif
    
    
#if SUPPORT_OPENGL
    // OpenGL case
    if (g_DeviceType == kGfxRendererOpenGL)
    {
        UserDataOpenGL* userDataOpenGL = (UserDataOpenGL*)frame->getUserData();
        glBindTexture(GL_TEXTURE_2D, userDataOpenGL->gpuTextureID);
        //glBindTexture(GL_TEXTURE_2D, tex);
    }
#endif
    
    while (!frame->getMutex().timed_lock(boost::get_system_time() + boost::posix_time::milliseconds(100)))
    {
        if (!alloServerProcess.isAlive())
        {
            // Reset the mutex otherwise it will block forever
            // Reset the condition otherwise it my be in inconsistent state
            // Hacky solution indeed
            void* mutexAddr     = &frame->getMutex();
            void* conditionAddr = &frame->getNewPixelsCondition();
            // That's not possible unfortunately since the mutex is locked and abandoned
            //face->getMutex().~interprocess_mutex();
            memset(mutexAddr, 0, sizeof(boost::interprocess::interprocess_mutex));
            memset(conditionAddr, 0, sizeof(boost::interprocess::interprocess_condition));
            new (mutexAddr)     boost::interprocess::interprocess_mutex;
            new (conditionAddr) boost::interprocess::interprocess_condition;
            return;
        }
    }

    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(frame->getMutex(),
                                                                                   boost::interprocess::accept_ownership);
    // COPY
    
#if SUPPORT_D3D9
    // D3D9 case
    if (g_DeviceType == kGfxRendererD3D9)
    {
        CubemapFaceD3D9* faceD3D9 = (CubemapFaceD3D9*)face;
        memcpy(faceD3D9->getPixels(),
			   faceD3D9->lockedRect.pBits,
			   faceD3D9->getWidth() * faceD3D9->getHeight() * 4);
    }
#endif
    
    
#if SUPPORT_D3D11
    // D3D11 case
    if (g_DeviceType == kGfxRendererD3D11)
    {
        CubemapFaceD3D11* faceD3D11 = (CubemapFaceD3D11*)face;
        memcpy(faceD3D11->getPixels(),
			   faceD3D11->resource.pData,
			   faceD3D11->getWidth() * faceD3D11->getHeight() * 4);
    }
#endif
    
    
#if SUPPORT_OPENGL
    // OpenGL case
    if (g_DeviceType == kGfxRendererOpenGL)
    {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->getPixels());
    }
#endif
    
    frame->getNewPixelsCondition().notify_all();
}

// --------------------------------------------------------------------------
// UnityRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.

extern "C" void EXPORT_API UnityRenderEvent (int eventID)
{
    // Allocate cubemap the first time we render a frame.
    // By doing so, we can make sure that both
    // the cubemap and the binoculars are fully configured.
    if (!thisProcess)
    {
        allocateSHM(cubemapConfig, binocularsConfig);
    }
    
    presentationTime = boost::chrono::system_clock::now();
    
    if (eventID == 1)
    {
        if (g_DeviceType == kGfxRendererD3D9 || g_DeviceType == kGfxRendererD3D11)
        {
            boost::thread* threads = new boost::thread[cubemap->getFacesCount()];

            for (int i = 0; i < cubemap->getFacesCount(); i++) {
                threads[i] = boost::thread(boost::bind(&copyFromGPUToCPU, cubemap->getFace(i)->getContent()));
            }

            for (int i = 0; i < cubemap->getFacesCount(); i++) {
                threads[i].join();
            }

        }
        else {
            for (int i = 0; i < cubemap->getFacesCount(); i++) {
                copyFromGPUToCPU(cubemap->getFace(i)->getContent());
            }
        }
    }
    else if (eventID == 2)
    {
        
    }
}

// --------------------------------------------------------------------------
// Start stop management

extern "C" void EXPORT_API ConfigureCubemapFromUnity(void** texturePtrs, int cubemapFacesCount, int resolution)
{
    
    
    /*glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    char* pixels = new char[resolution * resolution * 3];
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution, resolution, 0, GL_RGB, GL_BYTE, pixels);
    delete[] pixels;*/
    
    cubemapConfig              = new CubemapConfig;
    cubemapConfig->texturePtrs = texturePtrs;
    cubemapConfig->facesCount  = cubemapFacesCount;
    cubemapConfig->resolution  = resolution;
}

extern "C" void EXPORT_API ConfigureBinocularsFromUnity(void* texturePtr, int width, int height)
{
    binocularsConfig             = new BinocularsConfig;
    binocularsConfig->texturePtr = texturePtr;
    binocularsConfig->width      = width;
    binocularsConfig->height     = height;
}

extern "C" void EXPORT_API StopFromUnity()
{
    releaseSHM();
}