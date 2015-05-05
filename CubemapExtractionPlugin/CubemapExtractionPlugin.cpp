// Example low level rendering Unity plugin

#include <math.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>

#include "CubemapExtractionPlugin.h"
#include "PreviewWindow.h"
#include "CubemapFaceD3D9.h"
#include "CubemapFaceD3D11.h"

// --------------------------------------------------------------------------
// Helper utilities

Allocator<ShmSegmentManager>* shmAllocator = nullptr;

// Prints a string
static void DebugLog (const char* str)
{
	#if UNITY_WIN
	OutputDebugStringA (str);
	#else
	printf ("%s", str);
	#endif
}

boost::interprocess::managed_shared_memory shm;

void allocateCubemap()
{
	if (!cubemap)
	{
		boost::interprocess::shared_memory_object::remove("MySharedMemory");

		shm =
			boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create,
			"MySharedMemory",
			2048 * 2048 * 4 * 12 +
			sizeof(CubemapImpl) + 
			12 * sizeof(CubemapFace) +
			65536);

		shmAllocator = new Allocator<ShmSegmentManager>(shm.get_segment_manager());

		shm.destroy<CubemapImpl>("Cubemap");
		cubemap = shm.construct<CubemapImpl>("Cubemap")(*shmAllocator);
	}
}

// --------------------------------------------------------------------------
// Start stop management

extern "C" void EXPORT_API StartFromUnity()
{
	// Create and/or open shared memory
	allocateCubemap();

	// Open window for previewing cubemap
	CreatePreviewWindow();

	
	

	//Set size
	//shm.truncate(sizeof(FrameData));

	//Map the whole shared memory in this process
	//region = mapped_region(shm, read_write);


	//Write all the memory to 1
	//std::memset(region.get_address(), 2, region.get_size());

	//Get the address of the mapped region
	//void * addr = region.get_address();

	//Construct the shared structure in memory
	//data = new (addr)FrameData;
}

extern "C" void EXPORT_API StopFromUnity()
{
	// Close preview window
	DestroyPreviewWindow();
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

extern "C" void EXPORT_API SetCubemapFaceTextureFromUnity(void* texturePtr, int index)
{
	allocateCubemap();

	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).

#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
	CubemapFaceD3D9::Ptr face = CubemapFaceD3D9::create(
			(IDirect3DTexture9*)texturePtr,
			index,
			*shmAllocator);
		cubemap->setFace(CubemapFace::Ptr(face));
	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		CubemapFaceD3D11::Ptr face = CubemapFaceD3D11::create(
			(ID3D11Texture2D*)texturePtr,
			index,
			*shmAllocator);
		cubemap->setFace(CubemapFace::Ptr(face));
	}
#endif


#if SUPPORT_OPENGL
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		// to be implemented
	}
#endif
}

// --------------------------------------------------------------------------
// UnityRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.

extern "C" void EXPORT_API UnityRenderEvent (int eventID)
{

#if SUPPORT_D3D9 || SUPPORT_D3D11
	// D3D9 case
	//if (g_DeviceType == kGfxRendererD3D9 && multithreaded)
	//{
		boost::thread* threads = new boost::thread[cubemap->count()];

		for (int i = 0; i < cubemap->count(); i++) {
			threads[i] = boost::thread(boost::bind(&CubemapFace::copyFromGPUToCPU, cubemap->getFace(i).get()));
		}

		for (int i = 0; i < cubemap->count(); i++) {
			threads[i].join();
		}

	//}
	//else if (g_DeviceType == kGfxRendererD3D11 || !multithreaded) {
	//	for (int i = 0; i < cubemap->count(); i++) {
	//		cubemap->getFace(i)->copyFromGPUToCPU();
	//	}
	//}

	RepaintPreviewWindow();
#endif
}