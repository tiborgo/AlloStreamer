
/************************************************************************************
Filename    :   Win32_DirectXAppUtil.h
Content     :   D3D11 application/Window setup functionality for RoomTiny
Created     :   October 20th, 2014
Author      :   Tom Heath
Copyright   :   Copyright 2014 Oculus, Inc. All Rights reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/
#ifndef OVR_Win32_DirectXAppUtil_h
#define OVR_Win32_DirectXAppUtil_h

#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"

#include "Extras/OVR_Math.h"
#include "d3dcompiler.h"
#pragma comment(lib, "d3dcompiler.lib")
using namespace OVR;

#include <d3d11.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#ifndef VALIDATE
#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(NULL, (msg), "OculusRoomTiny", MB_ICONERROR | MB_OK); exit(-1); }
#endif
#define UNUSED(x)  x=x



//------------------------------------------------------------
struct DepthBuffer
{
    ID3D11DepthStencilView * TexDsv;

    DepthBuffer(ID3D11Device * Device, Sizei size, int sampleCount = 1)
    {
        DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT;
        D3D11_TEXTURE2D_DESC dsDesc;
        dsDesc.Width = size.w;
        dsDesc.Height = size.h;
        dsDesc.MipLevels = 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format = format;
        dsDesc.SampleDesc.Count = sampleCount;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags = 0;
        dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        ID3D11Texture2D * Tex;
        Device->CreateTexture2D(&dsDesc, NULL, &Tex);
        Device->CreateDepthStencilView(Tex, NULL, &TexDsv);
    }
};

//----------------------------------------------------------------
struct DataBuffer
{
    ID3D11Buffer * D3DBuffer;
    size_t         Size;

    DataBuffer(ID3D11Device * Device, D3D11_BIND_FLAG use, const void* buffer, size_t size) : Size(size)
    {
        D3D11_BUFFER_DESC desc;   memset(&desc, 0, sizeof(desc));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.BindFlags = use;
        desc.ByteWidth = (unsigned)size;
        D3D11_SUBRESOURCE_DATA sr;
        sr.pSysMem = buffer;
        sr.SysMemPitch = sr.SysMemSlicePitch = 0;
        Device->CreateBuffer(&desc, buffer ? &sr : NULL, &D3DBuffer);
    }
};
typedef DataBuffer DXVertexBuffer;
typedef DataBuffer DXIndexBuffer;

//---------------------------------------------------------------------
struct DirectX11
{
    HWND                     Window;
    bool                     Running;
    bool                     Key[256];
    OVR::Sizei               WinSize;
    ID3D11Device           * Device;
    ID3D11DeviceContext    * Context;
    IDXGISwapChain         * SwapChain;
    struct DepthBuffer     * MainDepthBuffer;
    ID3D11Texture2D        * BackBuffer;
    ID3D11RenderTargetView * BackBufferRT;
    #define                  UNIFORM_DATA_SIZE 2000  // Fixed size buffer for shader constants, before copied into buffer
    unsigned char            UniformData[UNIFORM_DATA_SIZE];
    struct DataBuffer      * UniformBufferGen;

    static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        DirectX11 *p = (DirectX11 *)GetWindowLongPtr(hWnd, 0);
        switch (Msg)
        {
        case WM_KEYDOWN:
            p->Key[wParam] = true;
            break;
        case WM_KEYUP:
            p->Key[wParam] = false;
            break;
        case WM_DESTROY:
            p->Running = false;
            break;
        default:
            return DefWindowProcW(hWnd, Msg, wParam, lParam); 
        }

        if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE])
            p->Running = false;

        return 0;
    }

    bool InitWindowAndDevice(HINSTANCE hinst, OVR::Recti vp, LPCWSTR title = nullptr)
    {
        Running = true;

        // Clear input
        for (int i = 0; i<256; i++) DIRECTX.Key[i] = false;

        WNDCLASSW wc; memset(&wc, 0, sizeof(wc));
        wc.lpszClassName = L"OVRAppWindow";
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = WindowProc;
        wc.cbWndExtra = sizeof(struct DirectX11 *);
        RegisterClassW(&wc);

        const DWORD wsStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME;
        RECT winSize = { 0, 0, vp.w, vp.h };
        AdjustWindowRect(&winSize, wsStyle, FALSE);
        Window = CreateWindowW(L"OVRAppWindow", (title ? title : L"OculusRoomTiny (DX11)"), wsStyle | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, winSize.right - winSize.left, winSize.bottom - winSize.top,
            NULL, NULL, hinst, NULL);
        if (!Window) return(false);
        SetWindowLongPtr(Window, 0, LONG_PTR(this));

        WinSize = vp.GetSize();

        IDXGIFactory * DXGIFactory;
        IDXGIAdapter * Adapter;
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory))))
            return(false);
        if (FAILED(DXGIFactory->EnumAdapters(0, &Adapter)))
            return(false);
        if (FAILED(D3D11CreateDevice(Adapter, Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
            NULL, 0, NULL, 0, D3D11_SDK_VERSION, &Device, NULL, &Context)))
            return(false);

        // Create swap chain
        DXGI_SWAP_CHAIN_DESC scDesc; memset(&scDesc, 0, sizeof(scDesc));
        scDesc.BufferCount = 2;
        scDesc.BufferDesc.Width = WinSize.w;
        scDesc.BufferDesc.Height = WinSize.h;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_B8G8R8A8_UNORM;
        scDesc.BufferDesc.RefreshRate.Numerator = 0;
        scDesc.BufferDesc.RefreshRate.Denominator = 1;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.OutputWindow = Window;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.Windowed = TRUE;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
        if (FAILED(DXGIFactory->CreateSwapChain(Device, &scDesc, &SwapChain)))               return(false);

        // Create backbuffer
        if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer))) return(false);
        if (FAILED(Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRT)))         return(false);

        // Main depth buffer
        MainDepthBuffer = new DepthBuffer(Device, Sizei(WinSize.w, WinSize.h));
        Context->OMSetRenderTargets(1, &BackBufferRT, MainDepthBuffer->TexDsv);

        // Buffer for shader constants
        UniformBufferGen = new DataBuffer(Device, D3D11_BIND_CONSTANT_BUFFER, NULL, UNIFORM_DATA_SIZE);
        Context->VSSetConstantBuffers(0, 1, &DIRECTX.UniformBufferGen->D3DBuffer);

        // Set a standard blend state, ie transparency from alpha
        D3D11_BLEND_DESC bm;
        memset(&bm, 0, sizeof(bm));
        bm.RenderTarget[0].BlendEnable = TRUE;
        bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bm.RenderTarget[0].SrcBlend = bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        bm.RenderTarget[0].DestBlend = bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        ID3D11BlendState * BlendState;
        Device->CreateBlendState(&bm, &BlendState);
        Context->OMSetBlendState(BlendState, NULL, 0xffffffff);

        // Set max frame latency to 1
        IDXGIDevice1* DXGIDevice1 = NULL;
        HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice1);
        if (FAILED(hr) | (DXGIDevice1 == NULL)) return(false);
        DXGIDevice1->SetMaximumFrameLatency(1);
        DXGIDevice1->Release();

        return(true);
    }

    void SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, struct DepthBuffer * depthbuffer, float R=0, float G=0, float B=0, float A=0)
    {
        float black[] = { R, G, B, A }; // Important that alpha=0, if want pixels to be transparent, for manual layers
        Context->OMSetRenderTargets(1, &rendertarget, depthbuffer->TexDsv);
        Context->ClearRenderTargetView(rendertarget, black);
        Context->ClearDepthStencilView(depthbuffer->TexDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    }

    void SetViewport(Recti vp)
    {
        D3D11_VIEWPORT D3Dvp;
        D3Dvp.Width = (float)vp.w;    D3Dvp.Height = (float)vp.h;
        D3Dvp.MinDepth = 0;           D3Dvp.MaxDepth = 1;
        D3Dvp.TopLeftX = (float)vp.x; D3Dvp.TopLeftY = (float)vp.y;
        Context->RSSetViewports(1, &D3Dvp);
    }

    bool HandleMessages(void)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // This is to provide a means to terminate after a maximum number of frames
        // to facilitate automated testing
        #ifdef MAX_FRAMES_ACTIVE 
            if (--maxFrames <= 0) 
                Running = false;
        #endif

        return Running;
    }

    void ReleaseWindow(HINSTANCE hinst)
    {
        DestroyWindow(Window);
        UnregisterClassW(L"OVRAppWindow", hinst);
    }
} static DIRECTX;

//------------------------------------------------------------
struct Texture
{
    ID3D11Texture2D            * Tex;
    ID3D11ShaderResourceView   * TexSv;
    ID3D11RenderTargetView     * TexRtv;
    Sizei                        Size;

    enum { AUTO_WHITE = 1, AUTO_WALL, AUTO_FLOOR, AUTO_CEILING, AUTO_GRID };
    Texture(bool rendertarget, Sizei size, int autoFillData = 0, int sampleCount = 1) : Size(size)
    {
        int mipLevels = autoFillData ? 8 : 1;  // We make 8 if generating
        D3D11_TEXTURE2D_DESC desc;
		desc.Width = size.w;
		desc.Height = size.h;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = sampleCount;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;//D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		
		if (rendertarget) desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

        DIRECTX.Device->CreateTexture2D(&desc, NULL, &Tex);
        DIRECTX.Device->CreateShaderResourceView(Tex, NULL, &TexSv);
        if (rendertarget) DIRECTX.Device->CreateRenderTargetView(Tex, NULL, &TexRtv);

		if (autoFillData) AutoFillTexture(autoFillData, mipLevels, size);
    }

    void AutoFillTexture(int autoFillData, int mipLevels, Sizei size)
    {
		
        DWORD pix[256 * 256];
        for (int j = 0; j < 256; j++)
        for (int i = 0; i < 256; i++)
        {
            switch (autoFillData)
            {
            case(AUTO_WALL) : pix[j * 256 + i] = (i<128 && j<128)?//(((j / 4 & 15) == 0) || (((i / 4 & 15) == 0) && ((((i / 4 & 31) == 0) ^ ((j / 4 >> 4) & 1)) == 0))) ?
                0xff3c3c3c : 0xffb4b4b4; break;
            case(AUTO_FLOOR) : pix[j * 256 + i] = (((i >> 7) ^ (j >> 7)) & 1) ? 0xffb4b4b4 : 0xff505050; break;
            case(AUTO_CEILING) : pix[j * 256 + i] = (i / 4 == 0 || j / 4 == 0) ? 0xff505050 : 0xffb4b4b4; break;
            case(AUTO_WHITE) : pix[j * 256 + i] = 0xffffffff;              break;
            case(AUTO_GRID) : pix[j * 256 + i] = (i<4) || (i>251) || (j<4) || (j>251) ? 0xffffffff : 0xff000000; break;
            default: pix[j * 256 + i] = 0xffffffff;              break;
            }
        }
        for (int level = 0; level < mipLevels; level++)
        {
            DIRECTX.Context->UpdateSubresource(Tex, level, NULL, (unsigned char *)pix, size.w * 4, size.h * 4);

            for (int j = 0; j < (size.h & ~1); j += 2)
            {
                uint8_t* psrc = (unsigned char *)pix + (size.w * j * 4);
                uint8_t* pdest = (unsigned char *)pix + (size.w * j);
                for (int i = 0; i < size.w >> 1; i++, psrc += 8, pdest += 4)
                {
                    pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[size.w * 4 + 0] + psrc[size.w * 4 + 4]) >> 2;
                    pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[size.w * 4 + 1] + psrc[size.w * 4 + 5]) >> 2;
                    pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[size.w * 4 + 2] + psrc[size.w * 4 + 6]) >> 2;
                    pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[size.w * 4 + 3] + psrc[size.w * 4 + 7]) >> 2;
                }
            }
            size.w >>= 1;  size.h >>= 1;
        }
    }

	void FillTexture(void* pixels, UINT width, UINT height){
		int mipLevels = 1;
		
		for (int level = 0; level < mipLevels; level++)
		{
			
			DIRECTX.Context->UpdateSubresource(Tex, level, NULL, (unsigned char *)pixels, width*4, height*4);

			/*for (int j = 0; j < (height & ~1); j += 2)
			{
				uint8_t* psrc = (unsigned char *)pixels + (width * j * 4);
				uint8_t* pdest = (unsigned char *)pixels + (width * j);
				for (int i = 0; i < width >> 1; i++, psrc += 8, pdest += 4)
				{
					pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[width * 4 + 0] + psrc[width * 4 + 4]) >> 2;
					pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[width * 4 + 1] + psrc[width * 4 + 5]) >> 2;
					pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[width * 4 + 2] + psrc[width * 4 + 6]) >> 2;
					pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[width * 4 + 3] + psrc[width * 4 + 7]) >> 2;
				}
			}
			width >>= 1;  height >>= 1;
		*/
		}
	}
};


//-----------------------------------------------------
enum { MAT_WRAP = 1, MAT_WIRE = 2, MAT_ZALWAYS = 4, MAT_NOCULL = 8 };
struct Material
{
    ID3D11VertexShader      * D3DVert;
    ID3D11PixelShader       * D3DPix;
    Texture                 * Tex;
    ID3D11InputLayout       * InputLayout;
    UINT                      VertexSize;
    ID3D11SamplerState      * SamplerState;
    ID3D11RasterizerState   * Rasterizer;
    ID3D11DepthStencilState * DepthState;

    Material(Texture * t, DWORD flags = MAT_WRAP, D3D11_INPUT_ELEMENT_DESC * vertexDesc = NULL, int numVertexDesc = 3,
             char* vertexShader = NULL, char* pixelShader = NULL, int vSize = 24) : Tex(t), VertexSize(vSize)
    {
        D3D11_INPUT_ELEMENT_DESC defaultVertexDesc[] = {
            { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "Color", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }, };

        // Use defaults if no shaders specified
        char* defaultVertexShaderSrc =
            "float4x4 ProjView;  float4 MasterCol;"
            "void main(in  float4 Position  : POSITION,    in  float4 Color : COLOR0, in  float2 TexCoord  : TEXCOORD0,"
            "          out float4 oPosition : SV_Position, out float4 oColor: COLOR0, out float2 oTexCoord : TEXCOORD0)"
            "{   oPosition = mul(ProjView, Position); oTexCoord = TexCoord; oColor = MasterCol*Color; }";
        char* defaultPixelShaderSrc =
            "Texture2D Texture   : register(t0); SamplerState Linear : register(s0); "
            "float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float2 TexCoord : TEXCOORD0) : SV_Target"
            "{   float4 TexCol = Texture.Sample(Linear, TexCoord);  "
            "    if (TexCol.a==0) clip(-1); return (Color * TexCol); }"; // If alpha = 0, don't draw

        if (!vertexDesc)   vertexDesc = defaultVertexDesc;
        if (!vertexShader) vertexShader = defaultVertexShaderSrc;
        if (!pixelShader)  pixelShader = defaultPixelShaderSrc;

        // Create vertex shader
        ID3DBlob * blobData;
        D3DCompile(vertexShader, strlen(vertexShader), 0, 0, 0, "main", "vs_4_0", 0, 0, &blobData, 0);
        DIRECTX.Device->CreateVertexShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), NULL, &D3DVert);

        // Create input layout
        DIRECTX.Device->CreateInputLayout(vertexDesc, numVertexDesc,
            blobData->GetBufferPointer(), blobData->GetBufferSize(), &InputLayout);
        blobData->Release();

        // Create pixel shader
        D3DCompile(pixelShader, strlen(pixelShader), 0, 0, 0, "main", "ps_4_0", 0, 0, &blobData, 0);
        DIRECTX.Device->CreatePixelShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), NULL, &D3DPix);
        blobData->Release();

        // Create sampler state
        D3D11_SAMPLER_DESC ss; memset(&ss, 0, sizeof(ss));
		ss.AddressU = ss.AddressV = ss.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_BORDER;//flags & MAT_WRAP ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_BORDER;
        ss.Filter = D3D11_FILTER_ANISOTROPIC;
        ss.MaxAnisotropy = 8;
        ss.MaxLOD = 15;
        DIRECTX.Device->CreateSamplerState(&ss, &SamplerState);

        // Create rasterizer
        D3D11_RASTERIZER_DESC rs; memset(&rs, 0, sizeof(rs));
        rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
        rs.CullMode = flags & MAT_NOCULL ? D3D11_CULL_NONE : D3D11_CULL_BACK;
        rs.FillMode = flags & MAT_WIRE ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        DIRECTX.Device->CreateRasterizerState(&rs, &Rasterizer);

        // Create depth state
        D3D11_DEPTH_STENCIL_DESC dss;
        memset(&dss, 0, sizeof(dss));
        dss.DepthEnable = true;
        dss.DepthFunc = flags & MAT_ZALWAYS ? D3D11_COMPARISON_ALWAYS : D3D11_COMPARISON_LESS;
        dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        DIRECTX.Device->CreateDepthStencilState(&dss, &DepthState);
    }
};

//----------------------------------------------------------------------
struct Vertex
{ 
    Vector3f  Pos;
    DWORD     C;
    float     U, V;
    Vertex() {};
    Vertex(Vector3f pos, DWORD c, float u, float v) : Pos(pos),C(c),U(u),V(v) {};
};

//-----------------------------------------------------------------------
struct TriangleSet
{
    int          numVertices, numIndices;
    Vertex       Vertices[2000]; // Note fixed maximum
    uint16_t     Indices[2000];
    TriangleSet()              { numVertices = numIndices = 0; }
    void AddVertex(Vertex v)   { Vertices[numVertices++] = v;  }
    void AddIndex(uint16_t a)  { Indices[numIndices++]   = a;  }
    void AddQuad(Vertex v0, Vertex v1, Vertex v2, Vertex v3)
    {
        int n = numVertices;
        AddVertex(v0);    AddVertex(v1);    AddVertex(v2); AddVertex(v3);
        AddIndex(n+0);    AddIndex(n+1);    AddIndex(n+2); AddIndex(n+3); 
        AddIndex(n+2);    AddIndex(n+1); 
    }
    void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, DWORD c)
    {
        Vector3f Vert[][2] = {
        Vector3f(x1, y2, z1), Vector3f(z1, x1), Vector3f(x2, y2, z1), Vector3f(z1, x2),
        Vector3f(x2, y2, z2), Vector3f(z2, x2), Vector3f(x1, y2, z2), Vector3f(z2, x1),
        Vector3f(x1, y1, z1), Vector3f(z1, x1), Vector3f(x2, y1, z1), Vector3f(z1, x2),
        Vector3f(x2, y1, z2), Vector3f(z2, x2), Vector3f(x1, y1, z2), Vector3f(z2, x1),
        Vector3f(x1, y1, z2), Vector3f(z2, y1), Vector3f(x1, y1, z1), Vector3f(z1, y1),
        Vector3f(x1, y2, z1), Vector3f(z1, y2), Vector3f(x1, y2, z2), Vector3f(z2, y2),
        Vector3f(x2, y1, z2), Vector3f(z2, y1), Vector3f(x2, y1, z1), Vector3f(z1, y1),
        Vector3f(x2, y2, z1), Vector3f(z1, y2), Vector3f(x2, y2, z2), Vector3f(z2, y2),
        Vector3f(x1, y1, z1), Vector3f(x1, y1), Vector3f(x2, y1, z1), Vector3f(x2, y1),
        Vector3f(x2, y2, z1), Vector3f(x2, y2), Vector3f(x1, y2, z1), Vector3f(x1, y2),
        Vector3f(x1, y1, z2), Vector3f(x1, y1), Vector3f(x2, y1, z2), Vector3f(x2, y1),
        Vector3f(x2, y2, z2), Vector3f(x2, y2), Vector3f(x1, y2, z2), Vector3f(x1, y2), };

        uint16_t CubeIndices[] = { 0, 1, 3, 3, 1, 2, 5, 4, 6, 6, 4, 7,
            8, 9, 11, 11, 9, 10, 13, 12, 14, 14, 12, 15,
            16, 17, 19, 19, 17, 18, 21, 20, 22, 22, 20, 23 };

        for (int i = 0; i < 36; i++)
            AddIndex(CubeIndices[i] + (uint16_t)numVertices);

        for (int v = 0; v < 24; v++)
        {   // Make vertices, with some token lighting
            Vertex vvv; vvv.Pos = Vert[v][0];  vvv.U = Vert[v][1].x; vvv.V = Vert[v][1].y;
            float dist1 = (vvv.Pos - Vector3f(-2, 4, -2)).Length();
            float dist2 = (vvv.Pos - Vector3f( 3, 4, -3)).Length();
            float dist3 = (vvv.Pos - Vector3f(-4, 3, 25)).Length();
            int   bri = rand() % 160;
            float R = ((c >> 16) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f; 
            float G = ((c >>  8) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
            float B = ((c >>  0) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
            vvv.C = (c & 0xff000000) + ((R>255?255:(DWORD)R)<<16) + ((G>255?255:(DWORD)G)<<8) + (B>255?255:(DWORD)B);
            AddVertex(vvv);
        }
    }
};

//----------------------------------------------------------------------
struct Model 
{
    Vector3f     Pos;
    Quatf        Rot;
    Material       * Fill;
    DXVertexBuffer * VertexBuffer;
    DXIndexBuffer  * IndexBuffer;
    int              NumIndices;

    Model(TriangleSet * t, Vector3f arg_pos, Material * arg_Fill) : Pos(arg_pos),Fill(arg_Fill)
    {
        NumIndices   = t->numIndices;
        VertexBuffer = new DataBuffer(DIRECTX.Device,D3D11_BIND_VERTEX_BUFFER, &t->Vertices[0], t->numVertices * sizeof(Vertex));
        IndexBuffer  = new DataBuffer(DIRECTX.Device,D3D11_BIND_INDEX_BUFFER, &t->Indices[0], t->numIndices * sizeof(t->Indices[0]));
    }

    void Render(Matrix4f projView, float R, float G, float B, float A, bool standardUniforms)
   {
        Matrix4f modelMat = Matrix4f::Translation(Pos) * Matrix4f(Rot);
        Matrix4f mat = (projView * modelMat).Transposed();
        float col[] = {R,G,B,A};  
        if (standardUniforms) memcpy(DIRECTX.UniformData + 0,  &mat, 64); // ProjView
        if (standardUniforms) memcpy(DIRECTX.UniformData + 64, &col, 16); // MasterCol
        D3D11_MAPPED_SUBRESOURCE map;
        DIRECTX.Context->Map(DIRECTX.UniformBufferGen->D3DBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
        memcpy(map.pData,  &DIRECTX.UniformData, UNIFORM_DATA_SIZE); 
        DIRECTX.Context->Unmap(DIRECTX.UniformBufferGen->D3DBuffer, 0);
        DIRECTX.Context->IASetInputLayout(Fill->InputLayout);
        DIRECTX.Context->IASetIndexBuffer(IndexBuffer->D3DBuffer, DXGI_FORMAT_R16_UINT, 0);
        UINT offset = 0;
        DIRECTX.Context->IASetVertexBuffers(0, 1, &VertexBuffer->D3DBuffer, &Fill->VertexSize, &offset);
        DIRECTX.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        DIRECTX.Context->VSSetShader(Fill->D3DVert, NULL, 0);
        DIRECTX.Context->PSSetShader(Fill->D3DPix, NULL, 0);
        DIRECTX.Context->PSSetSamplers(0, 1, &Fill->SamplerState);
        DIRECTX.Context->RSSetState(Fill->Rasterizer);
        DIRECTX.Context->OMSetDepthStencilState(Fill->DepthState, 0);
        DIRECTX.Context->PSSetShaderResources(0, 1, &Fill->Tex->TexSv);
        DIRECTX.Context->DrawIndexed((UINT)NumIndices, 0, 0);
    }

    Model(Texture * sourceTexture, float minx, float miny, float maxx, float maxy)
    {   // 2D scenes, for latency tester and full screen copies, etc
        TriangleSet quad;
        quad.AddQuad(Vertex(Vector3f(minx,miny,-10),0xffffffff,0,1),
                     Vertex(Vector3f(minx,maxy,-10),0xffffffff,0,0),
                     Vertex(Vector3f(maxx,miny,-10),0xffffffff,1,1),
                     Vertex(Vector3f(maxx,maxy,-10),0xffffffff,1,0));
        *this = Model(&quad,Vector3f(0, 0, 0), new Material(sourceTexture));
    }
};

//------------------------------------------------------------------------- 
struct Scene  
{
    int     num_models, Width,Height;
    Model * Models[12];

    void    Add(Model * n)
    {    Models[num_models++] = n; }    

    void Render(int eye,Matrix4f projView, float R, float G, float B, float A, bool standardUniforms)
	{
		for (int i = 0 + num_models / 2 * eye; i < num_models / 2 + num_models / 2 * eye; i++) Models[i]->Render(projView, R, G, B, A, standardUniforms);
	}

	void updateTextures(void *pixels[12], UINT w, UINT h){
		for (int i = 0; i < num_models; i++){
			if (pixels[i]!=NULL)
			Models[i]->Fill->Tex->FillTexture(pixels[i], w, h);
		}
		/*for (int i = num_models / 2; i < num_models; i++){

			Models[i]->Fill->Tex->AutoFillTexture(Texture::AUTO_WALL, 1, Sizei(256, 256));
		}*/
	}

	int getWidth(){
		return Width;
	}
	int getHeight(){
		return Height;
	}
	Scene(UINT width = 256, UINT height = 256, float focal_length = 10.0f) : num_models(0), Width(width), Height(height)
	{ 
		

        TriangleSet left,right,front,back,floor,ceiling;
		left.AddQuad(Vertex(Vector3f(-focal_length, -focal_length, focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(-focal_length, focal_length, focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(-focal_length, -focal_length, -focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(-focal_length, focal_length, -focal_length), 0xffffffff, 1, 0));
		right.AddQuad(Vertex(Vector3f(focal_length, -focal_length, -focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(focal_length, focal_length, -focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(focal_length, -focal_length, focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(focal_length, focal_length, focal_length), 0xffffffff, 1, 0));
		front.AddQuad(Vertex(Vector3f(-focal_length, -focal_length, -focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(-focal_length, focal_length, -focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(focal_length, -focal_length, -focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(focal_length, focal_length, -focal_length), 0xffffffff, 1, 0));
		back.AddQuad(Vertex(Vector3f(focal_length, -focal_length, focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(focal_length, focal_length, focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(-focal_length, -focal_length, focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(-focal_length, focal_length, focal_length), 0xffffffff, 1, 0));
		ceiling.AddQuad(Vertex(Vector3f(-focal_length, focal_length, -focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(-focal_length, focal_length, focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(focal_length, focal_length, -focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(focal_length, focal_length, focal_length), 0xffffffff, 1, 0));
		floor.AddQuad(Vertex(Vector3f(-focal_length, -focal_length, focal_length), 0xffffffff, 0, 1),
			Vertex(Vector3f(-focal_length, -focal_length, -focal_length), 0xffffffff, 0, 0),
			Vertex(Vector3f(focal_length, -focal_length, focal_length), 0xffffffff, 1, 1),
			Vertex(Vector3f(focal_length, -focal_length, -focal_length), 0xffffffff, 1, 0));
		
		Add(new Model(&right, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&left, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&front, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&back, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&ceiling, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&floor, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));

		Add(new Model(&right, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&left, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&front, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&back, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&ceiling, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
		Add(new Model(&floor, Vector3f(0, 0, 0), new Material(new Texture(false, Sizei(width, height)))));
	}
};

//-----------------------------------------------------------
struct Camera
{
    Vector3f Pos;
    Matrix4f Rot;
    Camera() { };
    Camera(Vector3f pos, Matrix4f rot) : Pos(pos), Rot(rot) { };
    Matrix4f GetViewMatrix()
    {    
        Vector3f finalUp      = Rot.Transform(Vector3f(0, 1, 0));
        Vector3f finalForward = Rot.Transform(Vector3f(0, 0, -1));
        return(Matrix4f::LookAtRH(Pos, Pos + finalForward, finalUp));
    }
};

//----------------------------------------------------
struct Utility
{
    void Output(const char * fnt,...)
    {
        static char string_text[1000];
        va_list args; va_start(args,fnt);
        vsprintf_s(string_text,fnt,args);
        va_end(args);
        OutputDebugStringA(string_text);
    }
} static Util;


struct OculusTexture
{
	ovrSwapTextureSet      * TextureSet;
	ID3D11RenderTargetView * TexRtv[3];

	OculusTexture(ovrHmd hmd, Sizei size)
	{
		D3D11_TEXTURE2D_DESC dsDesc;
		dsDesc.Width = size.w;
		dsDesc.Height = size.h;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		dsDesc.SampleDesc.Count = 1;   // No multi-sampling allowed
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;
		dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		if (ovrHmd_CreateSwapTextureSetD3D11(hmd, DIRECTX.Device, &dsDesc, &TextureSet) != ovrSuccess)
		{
			ovrErrorInfo errorInfo;
			ovr_GetLastErrorInfo(&errorInfo);
			std::cerr << errorInfo.ErrorString << std::endl;
			abort();
		}
		for (int i = 0; i < TextureSet->TextureCount; ++i)
		{
			ovrD3D11Texture* tex = (ovrD3D11Texture*)&TextureSet->Textures[i];
			DIRECTX.Device->CreateRenderTargetView(tex->D3D11.pTexture, NULL, &TexRtv[i]);
		}
	}

	void AdvanceToNextTexture()
	{
		if (TextureSet == NULL)
			std::cout << "null";
		TextureSet->CurrentIndex = (TextureSet->CurrentIndex + 1) % TextureSet->TextureCount;
	}
	void Release(ovrHmd hmd)
	{
		ovrHmd_DestroySwapTextureSet(hmd, TextureSet);
	}
};

#endif // OVR_Win32_DirectXAppUtil_h
