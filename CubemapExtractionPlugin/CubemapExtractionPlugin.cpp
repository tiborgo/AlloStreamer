// Example low level rendering Unity plugin

#include <math.h>
#include <stdio.h>
#include <boost/thread.hpp>

#include "CubemapExtractionPlugin.h"
#include "PreviewWindow.h"

// --------------------------------------------------------------------------
// Helper utilities


// Prints a string
static void DebugLog (const char* str)
{
	#if UNITY_WIN
	OutputDebugStringA (str);
	#else
	printf ("%s", str);
	#endif
}

// --------------------------------------------------------------------------
// Start stop management

extern "C" void EXPORT_API StartFromUnity()
{
	// Open window for previewing cubemap
	CreatePreviewWindow();
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
static void SetGraphicsDeviceD3D9 (IDirect3DDevice9* device, GfxDeviceEventType eventType);
#endif
#if SUPPORT_D3D11
static void SetGraphicsDeviceD3D11 (ID3D11Device* device, GfxDeviceEventType eventType);
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
		SetGraphicsDeviceD3D9 ((IDirect3DDevice9*)device, (GfxDeviceEventType)eventType);
	}
	#endif

	#if SUPPORT_D3D11
	// D3D11 device, remember device pointer and device type.
	// The pointer we get is ID3D11Device.
	if (deviceType == kGfxRendererD3D11)
	{
		DebugLog ("Set D3D11 graphics device\n");
		g_DeviceType = deviceType;
		SetGraphicsDeviceD3D11 ((ID3D11Device*)device, (GfxDeviceEventType)eventType);
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


// -------------------------------------------------------------------
//  Direct3D 9 setup/teardown code


#if SUPPORT_D3D9

static IDirect3DDevice9* g_D3D9Device;

static void SetGraphicsDeviceD3D9 (IDirect3DDevice9* device, GfxDeviceEventType eventType)
{
	g_D3D9Device = device;
}

#endif // #if SUPPORT_D3D9

// -------------------------------------------------------------------
//  Direct3D 11 setup/teardown code


#if SUPPORT_D3D11

static ID3D11Device* g_D3D11Device;

static void SetGraphicsDeviceD3D11 (ID3D11Device* device, GfxDeviceEventType eventType)
{
	g_D3D11Device = device;
}

#endif // #if SUPPORT_D3D11

// --------------------------------------------------------------------------

#if SUPPORT_D3D9

CubemapFaceD3D9::CubemapFaceD3D9(
	boost::uint32_t width,
	boost::uint32_t height,
	IDirect3DTexture9* texturePtr,
	IDirect3DSurface9* gpuSurfacePtr,
	IDirect3DSurface9* cpuSurfacePtr,
	D3DFORMAT format,
	D3DLOCKED_RECT lockedRect
	) :
	CubemapFace(width, height),
	texturePtr(texturePtr),
	gpuSurfacePtr(gpuSurfacePtr),
	cpuSurfacePtr(cpuSurfacePtr),
	format(format),
	lockedRect(lockedRect) {

}

CubemapFaceD3D9* CubemapFaceD3D9::create(IDirect3DTexture9* texturePtr) {

	D3DSURFACE_DESC textureDescription;
	HRESULT hr = texturePtr->GetLevelDesc(0, &textureDescription);

	UINT width = textureDescription.Width;
	UINT height = textureDescription.Height;
	D3DFORMAT format = textureDescription.Format;

	IDirect3DSurface9* gpuSurfacePtr;
	IDirect3DSurface9* cpuSurfacePtr;

	hr = texturePtr->GetSurfaceLevel(0, &gpuSurfacePtr);

	hr = g_D3D9Device->CreateOffscreenPlainSurface(width, height, format,
		D3DPOOL_SYSTEMMEM, &cpuSurfacePtr, NULL);

	D3DLOCKED_RECT lockedRect;
	ZeroMemory(&lockedRect, sizeof(D3DLOCKED_RECT));
	hr = cpuSurfacePtr->LockRect(&lockedRect, 0, D3DLOCK_READONLY);
	hr = cpuSurfacePtr->UnlockRect();

	return new CubemapFaceD3D9(width, height, texturePtr,
		gpuSurfacePtr, cpuSurfacePtr, textureDescription.Format, lockedRect);
}

#endif

extern "C" void EXPORT_API SetCubemapFaceCountFromUnity(int count)
{
	//cubemapFaceCount = count;
	//cubemapFaces = new CubemapFace*[count];
	//ZeroMemory(cubemapFaces, sizeof(CubemapFace*) * count);
}

extern "C" void EXPORT_API SetCubemapFaceTextureFromUnity(void* texturePtr, int face)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).

#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
		cubemap.setFace(face, CubemapFaceD3D9::create((IDirect3DTexture9*)texturePtr));
	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		/*CubemapFaceD3D11* cubemapFaceD3D11 = new CubemapFaceD3D11();

		cubemapFaceD3D11->gpuTexturePtr = (ID3D11Texture2D*)texturePtr;

		D3D11_TEXTURE2D_DESC textureDescription;
		cubemapFaceD3D11->gpuTexturePtr->GetDesc(&textureDescription);

		cubemapFaceD3D11->width = textureDescription.Width;
		cubemapFaceD3D11->height = textureDescription.Height;

		textureDescription.BindFlags = 0;
		textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDescription.Usage = D3D11_USAGE_STAGING;

		HRESULT hr = g_D3D11Device->CreateTexture2D(&textureDescription, NULL, &cubemapFaceD3D11->cpuTexturePtr);

		ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
		g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);
		//HRESULT hr2 = g_D3D11Device->CreateDeferredContext(0, &g_D3D11DeviceContext);

		ZeroMemory(&cubemapFaceD3D11->resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
		hr = g_D3D11DeviceContext->Map(cubemapFaceD3D11->cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &cubemapFaceD3D11->resource);
		g_D3D11DeviceContext->Unmap(cubemapFaceD3D11->cpuTexturePtr, subresource);

		// Assuming format D3DFMT_A8R8G8B8 (4 byte per pixel)
		cubemapFaceD3D11->pixels = new char[cubemapFaceD3D11->width * cubemapFaceD3D11->height * 4];

		cubemapFaces[face] = cubemapFaceD3D11;*/
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

void CopyFromGPUToCPU(CubemapFace* cubemapFace) {
	
#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
		CubemapFaceD3D9* cubemapFaceD3D9 = (CubemapFaceD3D9*)cubemapFace;

		// copy data from GPU to CPU
		HRESULT hr = g_D3D9Device->GetRenderTargetData(cubemapFaceD3D9->gpuSurfacePtr, cubemapFaceD3D9->cpuSurfacePtr);

		//ZeroMemory(&cubemapFaceD3D9->lockedRect, sizeof(D3DLOCKED_RECT));
		//hr = cubemapFaceD3D9->cpuSurfacePtr->LockRect(&cubemapFaceD3D9->lockedRect, 0, D3DLOCK_READONLY);

		memcpy(cubemapFaceD3D9->pixels, cubemapFaceD3D9->lockedRect.pBits, cubemapFaceD3D9->width * cubemapFaceD3D9->height * 4);

		//hr = cubemapFaceD3D9->cpuSurfacePtr->UnlockRect();

	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		CubemapFaceD3D11* cubemapFaceD3D11 = (CubemapFaceD3D11*)cubemapFace;

		ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
		g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);

		// copy data from GPU to CPU
		g_D3D11DeviceContext->CopyResource(cubemapFaceD3D11->cpuTexturePtr, cubemapFaceD3D11->gpuTexturePtr);
		
		//ZeroMemory(&cubemapFaceD3D11->resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
		//HRESULT hr = g_D3D11DeviceContext->Map(cubemapFaceD3D11->cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &cubemapFaceD3D11->resource);
		
		memcpy(cubemapFaceD3D11->pixels, cubemapFaceD3D11->resource.pData, cubemapFaceD3D11->width * cubemapFaceD3D11->height * 4);
		
		//g_D3D11DeviceContext->Unmap(cubemapFaceD3D11->cpuTexturePtr, subresource);
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

extern "C" void EXPORT_API UnityRenderEvent (int eventID)
{

#if SUPPORT_D3D9 || SUPPORT_D3D11
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9 && multithreaded)
	{
		boost::thread* threads = new boost::thread[cubemap.faces.size()];

		for (int i = 0; i < cubemap.faces.size(); i++) {
			threads[i] = boost::thread(boost::bind(&CopyFromGPUToCPU, cubemap.faces[i]));
		}

		for (int i = 0; i < cubemap.faces.size(); i++) {
			threads[i].join();
		}

	}
	else if (g_DeviceType == kGfxRendererD3D11 || !multithreaded) {
		for (int i = 0; i < cubemap.faces.size(); i++) {
			CopyFromGPUToCPU(cubemap.faces[i]);
		}
	}

	RepaintPreviewWindow();
#endif
}