#include <Windows.h>
#include <boost\thread.hpp>
#include <iostream>

#include "PreviewWindow.h"
#include "CubemapExtractionPlugin.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

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
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:

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
		L"CubemapExtractionPlugin", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	hWnd = CreateWindow(L"CubemapExtractionPlugin", L"Cubemap Extraction Plugin Preview",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);


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

	UnregisterClass(L"CubemapExtractionPlugin", wc.hInstance);
	return 0;
}



