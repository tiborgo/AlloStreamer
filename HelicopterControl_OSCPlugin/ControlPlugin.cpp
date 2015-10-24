// Example low level rendering Unity plugin

#include "UnityPluginInterface.h"

//#include <pthread.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>
#include <osc/OscOutboundPacketStream.h>
#include <ip/UdpSocket.h>
#include <boost/thread/barrier.hpp>

#include "AlloServer_Binoculars/FrameData.h"

#ifdef __cplusplus


extern "C" {
#endif
    
    
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <sys/types.h>
    
unsigned char *image = NULL;
    
    
FILE * pluginFile;
bool startedServer = false;
    // --------------------------------------------------------------------------
    // Include headers for the graphics APIs we support
}
#if SUPPORT_D3D9
	#include <d3d9.h>
#endif
#if SUPPORT_D3D11
	#include <d3d11.h>
#endif
#if SUPPORT_OPENGL
	#if UNITY_WIN
		#include <Windows.h>
		#include <GL/gl.h>
	#elif UNITY_LINUX
		#include <GL/gl.h>
	#elif UNITY_OSX
		#include <OpenGL/OpenGL.h>
		#include <OpenGL/gl.h>
	#endif
#endif



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
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time=1;

extern "C" void EXPORT_API SetTimeFromUnity (float t) { g_Time = t; }

#if SUPPORT_D3D9
IDirect3DDevice9* g_D3D9Device;
#endif
#if SUPPORT_D3D11
ID3D11Device* g_D3D11Device;
ID3D11Texture2D* g_D3D11GPUTexturePtr;
ID3D11Texture2D* g_D3D11CPUTexturePtr;
D3D11_MAPPED_SUBRESOURCE g_D3D11Resource;
#endif
#if SUPPORT_OPENGL
GLuint g_gltex;
#endif

int texWidth, texHeight;


static int g_DeviceType = -1;

// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void* g_TexturePointer;
unsigned int logCount=0;
unsigned char* pixels;

const int i_width = 1920;
const int i_height = 1080;

//const int i_width = 1024;
//const int i_height = 576;
extern "C" void EXPORT_API setLog()
{
    pluginFile = fopen(ROOT_DIR "/Logs/UnityServerPlugin.log", "w");
    fprintf(pluginFile, "Initializing interprocess memory...\n");
    fflush(pluginFile);
}
extern "C" void EXPORT_API SetTextureFromUnity (void* texturePtr)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
	g_TexturePointer = texturePtr;

#if SUPPORT_D3D9
	// D3D9 device, remember device pointer and device type.
	// The pointer we get is IDirect3DDevice9.
	if (g_DeviceType == kGfxRendererD3D9)
	{
		/*D3DSURFACE_DESC textureDescription;
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
		hr = cpuSurfacePtr->UnlockRect();*/
	}
#endif

#if SUPPORT_D3D11
	// D3D11 device, remember device pointer and device type.
	// The pointer we get is ID3D11Device.
	if (g_DeviceType == kGfxRendererD3D11)
	{
		g_D3D11GPUTexturePtr = (ID3D11Texture2D*)g_TexturePointer;

		D3D11_TEXTURE2D_DESC textureDescription;
		g_D3D11GPUTexturePtr->GetDesc(&textureDescription);

		texWidth = textureDescription.Width;
		texHeight = textureDescription.Height;

		textureDescription.BindFlags = 0;
		textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDescription.Usage = D3D11_USAGE_STAGING;

		HRESULT hr = g_D3D11Device->CreateTexture2D(&textureDescription, NULL, &g_D3D11CPUTexturePtr);

		ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
		g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);

		ZeroMemory(&g_D3D11Resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
		hr = g_D3D11DeviceContext->Map(g_D3D11CPUTexturePtr, subresource, D3D11_MAP_READ, 0, &g_D3D11Resource);
		g_D3D11DeviceContext->Unmap(g_D3D11CPUTexturePtr, subresource);
	}
#endif

#if SUPPORT_OPENGL
	// If we've got an OpenGL device, remember device type. There's no OpenGL
	// "device pointer" to remember since OpenGL always operates on a currently set
	// global context.
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		g_gltex = (GLuint)(size_t)(g_TexturePointer);
		glBindTexture(GL_TEXTURE_2D, g_gltex);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
	}
#endif
}



// --------------------------------------------------------------------------
// UnitySetGraphicsDevice



// Actual setup/teardown functions defined below


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

using namespace boost::interprocess;
mapped_region region;
shared_memory_object shm;
extern "C" void EXPORT_API UnityRenderEvent (int eventID)
{
	// When we use the RenderCubemap.cs and USeRenderingPlugin.cs scripts
	// This function will get called twice (with different eventIDs).
	// We have to make sure that the extraction only happens once for the cubemap
	// and binoculars respectively!
	if (eventID == 0)
	{
		// Unknown graphics device type? Do nothing.
		if (g_DeviceType == -1)
			return;
    
    
		// A colored triangle. Note that colors will come out differently
		// in D3D9/11 and OpenGL, for example, since they expect color bytes
		// in different ordering.
		MyVertex verts[3] = {
			{ -0.5f, -0.25f,  0, 0xFFff0000 },
			{  0.5f, -0.25f,  0, 0xFF00ff00 },
			{  0,     0.5f ,  0, 0xFF0000ff },
		};
    
    
		// Some transformation matrices: rotate around Z axis for world
		// matrix, identity view matrix, and identity projection matrix.
    
		float phi = g_Time;
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);
    
		float worldMatrix[16] = {
			cosPhi,-sinPhi,0,0,
			sinPhi,cosPhi,0,0,
			0,0,1,0,
			0,0,0.7f,1,
		};
		float identityMatrix[16] = {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1,
		};
		float projectionMatrix[16] = {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1,
		};
    
		// Actual functions defined below
		SetDefaultGraphicsState ();
		DoRendering (worldMatrix, identityMatrix, projectionMatrix, verts);
	}
}




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
		//ctx->OMSetDepthStencilState (g_D3D11DepthState, 0);
		//ctx->RSSetState (g_D3D11RasterState);
		//ctx->OMSetBlendState (g_D3D11BlendState, NULL, 0xFFFFFFFF);
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


FrameData *data;
static void DoRendering (const float* worldMatrix, const float* identityMatrix, float* projectionMatrix, const MyVertex* verts)
{
    fprintf(pluginFile, "DoRendering()");
    fflush(pluginFile);
	// Does actual rendering of a simple triangle
    
    //    fprintf(pluginFile, "%i: DoRendering()\n", logCount);
    //    fflush(pluginFile);
    logCount++;

#if SUPPORT_D3D9
	// D3D9 case
	if (g_DeviceType == kGfxRendererD3D9)
	{
		//CubemapFaceD3D9* faceD3D9 = (CubemapFaceD3D9*)face;
		// copy data from GPU to CPU
		//HRESULT hr = g_D3D9Device->GetRenderTargetData(faceD3D9->gpuSurfacePtr, faceD3D9->cpuSurfacePtr);

		/*CubemapFaceD3D9* faceD3D9 = (CubemapFaceD3D9*)face;
		memcpy(faceD3D9->getPixels(),
		faceD3D9->lockedRect.pBits,
		faceD3D9->getWidth() * faceD3D9->getHeight() * 4);*/
	}
#endif


#if SUPPORT_D3D11
	// D3D11 case
	if (g_DeviceType == kGfxRendererD3D11)
	{
		ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
		g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);

		// Copy data from GPU to CPU
		// Since the texture is YUV420p encoded we only need 3/8 the amount of data
		D3D11_BOX region =
		{
			0, 0, 0,
			texWidth, texHeight, 1
		};
		g_D3D11DeviceContext->CopySubresourceRegion(g_D3D11CPUTexturePtr, 0, 0, 0, 0, g_D3D11GPUTexturePtr, 0, &region);

		memcpy(data->pixels,
			g_D3D11Resource.pData,
			texWidth * texHeight * 4);
	}
#endif
    
#if SUPPORT_OPENGL
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
        fprintf(pluginFile, "Device is opengl");
        fflush(pluginFile);
        //        fprintf(pluginFile, "%i: OpenGL\n", logCount);
        //        fflush(pluginFile);
		// Transformation matrices
		glMatrixMode (GL_MODELVIEW);
		glLoadMatrixf (worldMatrix);
		glMatrixMode (GL_PROJECTION);
		// Tweak the projection matrix a bit to make it match what identity
		// projection would do in D3D case.
		projectionMatrix[10] = 2.0f;
		projectionMatrix[14] = -1.0f;
		glLoadMatrixf (projectionMatrix);
        
		// Vertex layout
		glVertexPointer (3, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
		glEnableClientState (GL_VERTEX_ARRAY);
		glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(verts[0]), &verts[0].color);
		glEnableClientState (GL_COLOR_ARRAY);
        
        //        fprintf(pluginFile, "%i: DrawArrays()\n", logCount);
        //        fflush(pluginFile);
        
		// Draw!
		//glDrawArrays (GL_TRIANGLES, 0, 3);
        
		// update native texture from code
		if (g_TexturePointer)
		{
            //            fprintf(pluginFile, "texture pointer: %u\n", g_TexturePointer);
            //            fflush(pluginFile);
            
			glBindTexture(GL_TEXTURE_2D, g_gltex);
            
//            glEnableClientState (GL_TEXTURE_COORD_ARRAY); 
//            glVertexPointer (3, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
//            glEnableClientState (GL_VERTEX_ARRAY);
//            glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(verts[0]), &verts[0].color);
//            //glEnableClientState (GL_COLOR_ARRAY);
//            
//            glDrawArrays (GL_TRIANGLES, 0, 3);
            
            //            fprintf(pluginFile, "%i: Set texture.\n", logCount);
            //            fflush(pluginFile);
            fprintf(pluginFile, "Tex width: %i  Tex height: %i\n", texWidth, texHeight);
			fflush(pluginFile);
            
            
            //unsigned char tempData[100*3] = {0};
            glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,data->pixels);
            
		}
	}
#endif
}


#define PORT 7245
#define QPORT 7244
#define SEND_PORT 7001
float startX = 0;
float startY = 0;
float endX = 0;
float endY = 0;
extern "C" {
	int EXPORT_API startStream();
	int EXPORT_API initInterprocessMemory();
	float EXPORT_API getStartX();
	float EXPORT_API getStartY();
	float EXPORT_API getEndX();
	float EXPORT_API getEndY();
	float EXPORT_API testFunc();
	void EXPORT_API setDistance(float distance);
	void EXPORT_API endServer();
	void EXPORT_API oscStart();
	void EXPORT_API oscPhaseSpaceStart();
}
extern "C" void EXPORT_API endServer()
{
    data->shutdownServer = true;
    fprintf(pluginFile, "Ending server...\n");
    fflush(pluginFile);
}

extern "C" float EXPORT_API getStartX()
{
	//DebugLog("Well there's this\n");
	return 56.2;
	//return startX;
}

extern "C" float EXPORT_API getStartY()
{
	return 45;
	//return startY;
}

extern "C" float EXPORT_API getEndX()
{
	return endX;
}

extern "C" float EXPORT_API getEndY()
{
	return endY;
}

extern "C" float EXPORT_API testFunc()
{
	return 199;
}

UdpTransmitSocket* transmitSocket = nullptr;

extern "C" void EXPORT_API setDistance(float distance)
{
	if (!transmitSocket)
	{
		transmitSocket = new UdpTransmitSocket(IpEndpointName("192.168.1.183", SEND_PORT));
	}
	
	const int OUTPUT_BUFFER_SIZE = 64;
	char buffer[OUTPUT_BUFFER_SIZE];
	osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

	p << osc::BeginBundleImmediate
		<< osc::BeginMessage("/distance")
		<< distance << osc::EndMessage
		<< osc::EndBundle;
	transmitSocket->Send(p.Data(), p.Size());
}

//OSC START
class QuaternionPacketListener : public osc::OscPacketListener {
protected:
    
    virtual void ProcessMessage( const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint )
    {
        (void) remoteEndpoint; // suppress unused parameter warning
        
        try{
            // example of parsing single messages. osc::OsckPacketListener
            // handles the bundle traversal.
            
            // example #2 -- argument iterator interface, supports
            // reflection for overloaded messages (eg you can call
            // (*arg)->IsBool() to check if a bool was passed etc).
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            startX = (arg++)->AsFloat();
            startY = (arg++)->AsFloat();
            endX = (arg++)->AsFloat();
            endY = (arg++)->AsFloat();
            
            if( arg != m.ArgumentsEnd() )
                throw osc::ExcessArgumentException();
            
        }catch( osc::Exception& e ){
            // any parsing errors such as unexpected argument types, or
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
            << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};
//OSC END

UdpListeningReceiveSocket* s = NULL;
UdpListeningReceiveSocket* qs = NULL;
//UdpListeningReceiveSocket s;

boost::barrier sBreakBarrier(2);
boost::barrier qsBreakBarrier(2);

//OSC START
extern "C" void EXPORT_API oscStartHelicopterControl()
{
	/*
     * Start OSC client to receive phone sensor orientation
     */
	QuaternionPacketListener listener;
    s = new UdpListeningReceiveSocket(IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ), &listener );
    
    s->Run();
	sBreakBarrier.wait();
}
//OSC END

extern "C" void EXPORT_API oscPhaseSpaceStartHelicopterControl()
{
	/*
     * Start OSC client to receive Phase Space sensor orientation
     */
    QuaternionPacketListener qlistener;
    qs = new UdpListeningReceiveSocket(IpEndpointName( IpEndpointName::ANY_ADDRESS, QPORT ), &qlistener );
    
    qs->Run();
	qsBreakBarrier.wait();
}

unsigned char testPixels[i_width*i_height*3];
extern "C" int EXPORT_API initInterprocessMemory()
{
    

    
	/*
     * Initialize shared memory between Unity plugin and the Live 555 server/encoder
     */
    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("MySharedMemory2"); }
        ~shm_remove(){ shared_memory_object::remove("MySharedMemory2"); }
    } remover;
    
    //Create a shared memory object.
    shm = shared_memory_object(create_only, "MySharedMemory2", read_write);
    
    //Set size
    shm.truncate(sizeof(FrameData));
   
    //Map the whole shared memory in this process
    region = mapped_region(shm, read_write);
    
    
    //Write all the memory to 1
    //std::memset(region.get_address(), 2, region.get_size());
    
    //Get the address of the mapped region
    void * addr       = region.get_address();
    
    //Construct the shared structure in memory
    data = new (addr) FrameData;


    

    
//    for(int i=0; i<2048*2048*3; i++)
//    {
//        data->pixels[i] = rand() % 255;
//    }

	/*
     * Start server process
     * Will not return until endServer() is called
     */
    
	if (0 != std::system(ROOT_DIR "/Bin/" CMAKE_INTDIR "/AlloServer_Binoculars"))
        return 1;
    
    //OSC START
    //AlloServer is finished, so shutdown OSC
    if(s != NULL)
    {
        s->AsynchronousBreak();
		sBreakBarrier.wait();
        delete s;
        s = NULL;
    }
    //OSC END

    if(qs != NULL)
    {
        qs->AsynchronousBreak();
		qsBreakBarrier.wait();
        delete qs;
        qs = NULL;
    }
    
    fprintf(pluginFile,"AlloServer exited\n");
    fflush(pluginFile);
    
    
    return 0;
}


