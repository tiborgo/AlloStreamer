// Example low level rendering Unity plugin

#include <math.h>
#include <stdio.h>
#include <boost/thread.hpp>

#include "CubemapExtractionPlugin.h"
#include "PreviewWindow.h"

int cubemapFaceCount;
CubemapFace** cubemapFaces;


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

// COM-like Release macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a = NULL; }
#endif



// --------------------------------------------------------------------------
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time;

extern "C" void EXPORT_API SetTimeFromUnity (float t) { g_Time = t; }







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

	// Open window for previewing cubemap
	boost::thread previewWindowThread(&_WinMain);
}


// -------------------------------------------------------------------
//  Direct3D 9 setup/teardown code


#if SUPPORT_D3D9

static IDirect3DDevice9* g_D3D9Device;

// A dynamic vertex buffer just to demonstrate how to handle D3D9 device resets.
static IDirect3DVertexBuffer9* g_D3D9DynamicVB;

static void SetGraphicsDeviceD3D9 (IDirect3DDevice9* device, GfxDeviceEventType eventType)
{
	g_D3D9Device = device;

	// Create or release a small dynamic vertex buffer depending on the event type.
	switch (eventType) {
	case kGfxDeviceEventInitialize:
	case kGfxDeviceEventAfterReset:
		// After device is initialized or was just reset, create the VB.
		if (!g_D3D9DynamicVB)
			g_D3D9Device->CreateVertexBuffer (1024, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_D3D9DynamicVB, NULL);
		break;
	case kGfxDeviceEventBeforeReset:
	case kGfxDeviceEventShutdown:
		// Before device is reset or being shut down, release the VB.
		SAFE_RELEASE(g_D3D9DynamicVB);
		break;
	}
}

#endif // #if SUPPORT_D3D9



// -------------------------------------------------------------------
//  Direct3D 11 setup/teardown code


#if SUPPORT_D3D11

static ID3D11Device* g_D3D11Device;
static ID3D11Buffer* g_D3D11VB; // vertex buffer
static ID3D11Buffer* g_D3D11CB; // constant buffer
static ID3D11VertexShader* g_D3D11VertexShader;
static ID3D11PixelShader* g_D3D11PixelShader;
static ID3D11InputLayout* g_D3D11InputLayout;
static ID3D11RasterizerState* g_D3D11RasterState;
static ID3D11BlendState* g_D3D11BlendState;
static ID3D11DepthStencilState* g_D3D11DepthState;

typedef HRESULT (WINAPI *D3DCompileFunc)(
	const void* pSrcData,
	unsigned long SrcDataSize,
	const char* pFileName,
	const D3D10_SHADER_MACRO* pDefines,
	ID3D10Include* pInclude,
	const char* pEntrypoint,
	const char* pTarget,
	unsigned int Flags1,
	unsigned int Flags2,
	ID3D10Blob** ppCode,
	ID3D10Blob** ppErrorMsgs);

static const char* kD3D11ShaderText =
"cbuffer MyCB : register(b0) {\n"
"	float4x4 worldMatrix;\n"
"}\n"
"void VS (float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position) {\n"
"	opos = mul (worldMatrix, float4(pos,1));\n"
"	ocolor = color;\n"
"}\n"
"float4 PS (float4 color : COLOR) : SV_TARGET {\n"
"	return color;\n"
"}\n";


static void CreateD3D11Resources()
{
	D3D11_BUFFER_DESC desc;
	memset (&desc, 0, sizeof(desc));

	// vertex buffer
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 1024;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	g_D3D11Device->CreateBuffer (&desc, NULL, &g_D3D11VB);

	// constant buffer
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = 64; // hold 1 matrix
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	g_D3D11Device->CreateBuffer (&desc, NULL, &g_D3D11CB);

	// shaders
	HMODULE compiler = LoadLibraryA("D3DCompiler_43.dll");

	if (compiler == NULL)
	{
		// Try compiler from Windows 8 SDK
		compiler = LoadLibraryA("D3DCompiler_46.dll");
	}
	if (compiler)
	{
		ID3D10Blob* vsBlob = NULL;
		ID3D10Blob* psBlob = NULL;

		D3DCompileFunc compileFunc = (D3DCompileFunc)GetProcAddress (compiler, "D3DCompile");
		if (compileFunc)
		{
			HRESULT hr;
			hr = compileFunc(kD3D11ShaderText, strlen(kD3D11ShaderText), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &vsBlob, NULL);
			if (SUCCEEDED(hr))
			{
				g_D3D11Device->CreateVertexShader (vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &g_D3D11VertexShader);
			}

			hr = compileFunc(kD3D11ShaderText, strlen(kD3D11ShaderText), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, &psBlob, NULL);
			if (SUCCEEDED(hr))
			{
				g_D3D11Device->CreatePixelShader (psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &g_D3D11PixelShader);
			}
		}

		// input layout
		if (g_D3D11VertexShader && vsBlob)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			g_D3D11Device->CreateInputLayout (layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_D3D11InputLayout);
		}

		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);

		FreeLibrary (compiler);
	}
	else
	{
		DebugLog ("D3D11: HLSL shader compiler not found, will not render anything\n");
	}

	// render states
	D3D11_RASTERIZER_DESC rsdesc;
	memset (&rsdesc, 0, sizeof(rsdesc));
	rsdesc.FillMode = D3D11_FILL_SOLID;
	rsdesc.CullMode = D3D11_CULL_NONE;
	rsdesc.DepthClipEnable = TRUE;
	g_D3D11Device->CreateRasterizerState (&rsdesc, &g_D3D11RasterState);

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	memset (&dsdesc, 0, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	g_D3D11Device->CreateDepthStencilState (&dsdesc, &g_D3D11DepthState);

	D3D11_BLEND_DESC bdesc;
	memset (&bdesc, 0, sizeof(bdesc));
	bdesc.RenderTarget[0].BlendEnable = FALSE;
	bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
	g_D3D11Device->CreateBlendState (&bdesc, &g_D3D11BlendState);
}

static void ReleaseD3D11Resources()
{
	SAFE_RELEASE(g_D3D11VB);
	SAFE_RELEASE(g_D3D11CB);
	SAFE_RELEASE(g_D3D11VertexShader);
	SAFE_RELEASE(g_D3D11PixelShader);
	SAFE_RELEASE(g_D3D11InputLayout);
	SAFE_RELEASE(g_D3D11RasterState);
	SAFE_RELEASE(g_D3D11BlendState);
	SAFE_RELEASE(g_D3D11DepthState);
}

static void SetGraphicsDeviceD3D11 (ID3D11Device* device, GfxDeviceEventType eventType)
{
	g_D3D11Device = device;

	if (eventType == kGfxDeviceEventInitialize)
		CreateD3D11Resources();
	if (eventType == kGfxDeviceEventShutdown)
		ReleaseD3D11Resources();
}

#endif // #if SUPPORT_D3D11



// --------------------------------------------------------------------------
// SetDefaultGraphicsState
//
// Helper function to setup some "sane" graphics state. Rendering state
// upon call into our plugin can be almost completely arbitrary depending
// on what was rendered in Unity before.
// Before calling into the plugin, Unity will set shaders to null,
// and will unbind most of "current" objects (e.g. VBOs in OpenGL case).
//
// Here, we set culling off, lighting off, alpha blend & test off, Z
// comparison to less equal, and Z writes off.

static void SetDefaultGraphicsState ()
{
	#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
		g_D3D9Device->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);
		g_D3D9Device->SetRenderState (D3DRS_LIGHTING, FALSE);
		g_D3D9Device->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
		g_D3D9Device->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
		g_D3D9Device->SetRenderState (D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		g_D3D9Device->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);
	}
	#endif


	#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		ID3D11DeviceContext* ctx = NULL;
		g_D3D11Device->GetImmediateContext (&ctx);
		ctx->OMSetDepthStencilState (g_D3D11DepthState, 0);
		ctx->RSSetState (g_D3D11RasterState);
		ctx->OMSetBlendState (g_D3D11BlendState, NULL, 0xFFFFFFFF);
		ctx->Release();
	}
	#endif


	#if SUPPORT_OPENGL
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_BLEND);
		glDisable (GL_ALPHA_TEST);
		glDepthFunc (GL_LEQUAL);
		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_FALSE);
	}
	#endif
}

// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

extern "C" void EXPORT_API SetCubemapFaceCountFromUnity(int count)
{
	cubemapFaceCount = count;
	cubemapFaces = new CubemapFace*[count];
	ZeroMemory(cubemapFaces, sizeof(CubemapFace*) * count);
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
		CubemapFaceD3D9* cubemapFaceD3D9 = new CubemapFaceD3D9();

		cubemapFaceD3D9->texturePtr = (IDirect3DTexture9*)texturePtr;

		D3DSURFACE_DESC textureDescription;
		HRESULT hr = cubemapFaceD3D9->texturePtr->GetLevelDesc(0, &textureDescription);
		cubemapFaceD3D9->width = textureDescription.Width;
		cubemapFaceD3D9->height = textureDescription.Height;
		cubemapFaceD3D9->format = textureDescription.Format;

		hr = cubemapFaceD3D9->texturePtr->GetSurfaceLevel(0, &cubemapFaceD3D9->gpuSurfacePtr);

		hr = g_D3D9Device->CreateOffscreenPlainSurface(cubemapFaceD3D9->width,
			cubemapFaceD3D9->height,
			cubemapFaceD3D9->format,
			D3DPOOL_SYSTEMMEM,
			&cubemapFaceD3D9->cpuSurfacePtr,
			NULL);

		ZeroMemory(&cubemapFaceD3D9->lockedRect, sizeof(D3DLOCKED_RECT));
		hr = cubemapFaceD3D9->cpuSurfacePtr->LockRect(&cubemapFaceD3D9->lockedRect, 0, D3DLOCK_READONLY);
		hr = cubemapFaceD3D9->cpuSurfacePtr->UnlockRect();

		// Assuming format D3DFMT_A8R8G8B8 (4 byte per pixel)
		cubemapFaceD3D9->pixels = new char[cubemapFaceD3D9->width * cubemapFaceD3D9->height * 4];

		cubemapFaces[face] = cubemapFaceD3D9;
	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		CubemapFaceD3D11* cubemapFaceD3D11 = new CubemapFaceD3D11();

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

		cubemapFaces[face] = cubemapFaceD3D11;
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


struct MyVertex {
	float x, y, z;
	unsigned int color;
};
static void SetDefaultGraphicsState ();
static void DoRendering (const float* worldMatrix, const float* identityMatrix, float* projectionMatrix, const MyVertex* verts);

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
		boost::thread* threads = new boost::thread[cubemapFaceCount];

		for (int i = 0; i < cubemapFaceCount; i++) {
			threads[i] = boost::thread(boost::bind(&CopyFromGPUToCPU, cubemapFaces[i]));
		}

		for (int i = 0; i < cubemapFaceCount; i++) {
			threads[i].join();
		}

	}
	else if (g_DeviceType == kGfxRendererD3D11 || !multithreaded) {
		for (int i = 0; i < cubemapFaceCount; i++) {
			CopyFromGPUToCPU(cubemapFaces[i]);
		}
	}

	repaint();
#endif
			//memcpy(out + (desc.Width * desc.Height * 4 * j), buffer, desc.Width * desc.Height * 4);

		

			/*for (int i = 0; i < desc.Width * desc.Height * 4; i++) {
				int index = (desc.Width * 4 * j) + ((desc.Width * 6 * 4) * (i / (desc.Width * 4))) + (i % (desc.Width * 4));
				out[index] = buffer[i];
			}*/

	

		/*for (int i = 0; i < desc.Width * desc.Height * 6; i++) {
			out[i*4] = 255;
		}*/

		//*pixels = out;
}