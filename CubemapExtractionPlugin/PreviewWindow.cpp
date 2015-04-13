//-----------------------------------------------------------------------------
// File: CreateDevice.cpp
//
// Desc: This is the first tutorial for using Direct3D. In this tutorial, all
//       we are doing is creating a Direct3D device and using it to clear the
//       window.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <d3d9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

#include <boost\thread.hpp>
#include <iostream>

#include "PreviewWindow.h"
#include "CubemapExtractionPlugin.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL; // Our rendering device

struct CubeMapFacePosition {
	float horizontal;
	float vertical;
};

float horizontalTilesCount = 4;
float verticalTilesCount = 3;

static CubeMapFacePosition cubeMapFacePositions[6] {
	{ 2.0f / horizontalTilesCount, 1.0f / verticalTilesCount },	// PositiveX
	{ 0.0f / horizontalTilesCount, 1.0f / verticalTilesCount },	// NegativeX
	{ 1.0f / horizontalTilesCount, 0.0f / verticalTilesCount },	// PositiveY
	{ 1.0f / horizontalTilesCount, 2.0f / verticalTilesCount },	// NegativeY
	{ 1.0f / horizontalTilesCount, 1.0f / verticalTilesCount },	// PositiveZ
	{ 3.0f / horizontalTilesCount, 1.0f / verticalTilesCount },	// NegativeZ
};

static HWND hWnd;

void repaint() {
	InvalidateRect(hWnd, NULL, TRUE);
}

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Create the D3D object, which is needed to create the D3DDevice.
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// Set up the structure used to create the D3DDevice. Most parameters are
	// zeroed out. We set Windowed to TRUE, since we want to do D3D in a
	// window, and then set the SwapEffect to "discard", which is the most
	// efficient method of presenting the back buffer to the display.  And 
	// we request a back buffer format that matches the current desktop display 
	// format.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// Create the Direct3D device. Here we are using the default adapter (most
	// systems only have one, unless they have multiple graphics hardware cards
	// installed) and requesting the HAL (which is saying we want the hardware
	// device rather than a software one). Software vertex processing is 
	// specified since we know it will work on all cards. On cards that support 
	// hardware vertex processing, though, we would see a big performance gain 
	// by specifying hardware vertex processing.
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// Device state would normally be set here

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)
		return;

	// Clear the backbuffer to a blue color
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// Begin the scene
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		// Rendering of scene objects can happen here

		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

//static void* dwBytes = new char[2048 * 2048 * 4];


void StretchBltThreadRun(HDC hMemDC, HDC hDC, RECT clientRect, LONG wWidth, LONG wHeight, int i) {

	HDC hFaceMemDC = CreateCompatibleDC(hDC);

	BITMAPINFO faceMemBMI;

	ZeroMemory(&faceMemBMI, sizeof(BITMAPINFO));

	faceMemBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	faceMemBMI.bmiHeader.biBitCount = 32;
	faceMemBMI.bmiHeader.biHeight = wHeight;
	faceMemBMI.bmiHeader.biWidth = wWidth;
	faceMemBMI.bmiHeader.biPlanes = 1;

	VOID* dwBytes;

	HBITMAP hFaceMemBitmap = CreateDIBSection(hFaceMemDC, &faceMemBMI, DIB_RGB_COLORS,
		&dwBytes, NULL, 0);

	SelectObject(hFaceMemDC, hFaceMemBitmap);
	memcpy(dwBytes, cubemapFaces[i]->pixels, wWidth * wHeight * 4);

	StretchBlt(hMemDC,
		clientRect.right * cubeMapFacePositions[i].horizontal,
		clientRect.bottom * cubeMapFacePositions[i].vertical,
		clientRect.right / horizontalTilesCount, clientRect.bottom / verticalTilesCount,
		hFaceMemDC, 0, 0, wWidth, wHeight, SRCCOPY);

	DeleteObject(hFaceMemBitmap);
	DeleteDC(hFaceMemDC);
}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		//Render();


		//ValidateRect(hWnd, NULL);

		if (cubemapFaceCount >= 6) {

			int wWidth = cubemapFaces[0]->width;
			int wHeight = cubemapFaces[0]->height;

			PAINTSTRUCT ps = { 0 };

			HDC hDC = BeginPaint(hWnd, &ps);

			RECT clientRect;
			GetClientRect(hWnd, &clientRect);

			HDC hMemDC = CreateCompatibleDC(hDC);

			

			BITMAPINFO memBMI;

			ZeroMemory(&memBMI, sizeof(BITMAPINFO));

			memBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			memBMI.bmiHeader.biBitCount = 32;
			memBMI.bmiHeader.biHeight = clientRect.bottom;
			memBMI.bmiHeader.biWidth = clientRect.right;
			memBMI.bmiHeader.biPlanes = 1;

			HBITMAP hMemBitmap = CreateDIBSection(hMemDC, &memBMI, DIB_RGB_COLORS,
				NULL, NULL, 0);

			SetStretchBltMode(hMemDC, HALFTONE);

			SelectObject(hMemDC, hMemBitmap);

			boost::thread threads[6];

			for (int i = 0; i < 6; i++) {

				
				
				
				/*BitBlt(hMemDC,
					cubeMapFacePositions[i].horizontal * wWidth,
					cubeMapFacePositions[i].vertical * wHeight,
					wWidth, wHeight, hFaceMemDC, 0, 0, SRCCOPY);*/
				/*StretchBlt(hMemDC,
					clientRect.right * cubeMapFacePositions[i].horizontal,
					clientRect.bottom * cubeMapFacePositions[i].vertical,
					clientRect.right / horizontalTilesCount, clientRect.bottom / verticalTilesCount,
					hFaceMemDC, 0, 0, wWidth, wHeight, SRCCOPY);*/

				StretchBltThreadRun(hMemDC, hDC, clientRect, wWidth, wHeight, i);

				//threads[i] = boost::thread(&StretchBltThreadRun, hMemDC, hDC, clientRect, wWidth, wHeight, i);
			}

			for (int i = 0; i < 6; i++) {
				threads[i].join();
			}

			//StretchBlt(hDC, 0, 0, clientRect.right, clientRect.bottom,
				//hMemDC, 0, 0, wWidth * 4, wHeight * 3, SRCCOPY);

			//SetDIBits(hDC, hBitmap, 0, wHeight, cubemapFaces[0].pixels, &bmi, DIB_RGB_COLORS);

			

			BitBlt(hDC, 0, 0, clientRect.right, clientRect.bottom, hMemDC, 0, 0, SRCCOPY);
			

			//SelectObject(hMemDC, hBitmap);

			DeleteObject(hMemBitmap);
			DeleteDC(hMemDC);
			EndPaint(hWnd, &ps);
		}
		
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: wWinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI _WinMain(/*HINSTANCE hInst, HINSTANCE, LPWSTR, INT*/)
{
	//UNREFERENCED_PARAMETER(hInst);

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 01: CreateDevice",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);





	// Initialize Direct3D
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		// Show the window
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		// Enter the message loop
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnregisterClass(L"D3D Tutorial", wc.hInstance);
	return 0;
}



