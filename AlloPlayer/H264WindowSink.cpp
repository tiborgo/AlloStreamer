

#include <iostream>
#include <map>
#include "H264WindowSink.h"
#include <boost/thread.hpp>
#include <GroupsockHelper.hh>

static std::map<HWND, H264WindowSink*> hwndThisMap;

H264WindowSink* H264WindowSink::createNew(UsageEnvironment& env,
	unsigned int bufferSize,
	MediaSubsession& subSession)
{
	return new H264WindowSink(env, bufferSize, subSession);
}

H264WindowSink::H264WindowSink(UsageEnvironment& env,
	unsigned int bufferSize,
	MediaSubsession& subSession)
	: MediaSink(env), bufferSize(bufferSize), buffer(new unsigned char[bufferSize]),
	subSession(subSession), img_convert_ctx(NULL)
{
	for (int i = 0; i < 1; i++)
	{
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		frame->format = AV_PIX_FMT_BGRA;

		framePool.push(frame);
	}
	

	// Initialize codec and decoder
	AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		fprintf(stderr, "Codec not found\n");
		return;
	}

	codecContext = avcodec_alloc_context3(codec);

	if (!codecContext)
	{
		fprintf(stderr, "could not allocate video codec context\n");
		return;
	}

	/* open it */
	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		fprintf(stderr, "could not open codec\n");
		return;
	}

	boost::thread windowThread(boost::bind(&H264WindowSink::windowLoop, this));
}

void H264WindowSink::afterGettingFrame(unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime)
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	long relativePresentationTimeMicroSec = presentationTime.tv_sec * 1000000 + presentationTime.tv_usec -
		(currentTime.tv_sec * 1000000 + currentTime.tv_usec);

	long acceptedDelayMicroSec = 2000;

	std::cout << this << " " << -relativePresentationTimeMicroSec << " microseconds to late" << std::endl;

	if (relativePresentationTimeMicroSec + acceptedDelayMicroSec >= 0)
	{



		u_int8_t nal_unit_type;
		if (frameSize >= 1)
		{
			nal_unit_type = buffer[0] & 0x1F;



			if (nal_unit_type == 8) // PPS
			{
				//envir() << "PPS seen; size: " << frameSize << "\n";
			}
			else if (nal_unit_type == 7) // SPS
			{
				//envir() << "SPS seen; size: " << frameSize << "\n";
			}
			else
			{
				//envir() << nal_unit_type << " seen; size: " << frameSize << "\n";
			}
		}


		//envir() << "sprop - parameter - sets: " << subSession.fmtp_spropparametersets() << "\n";
		unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
		unsigned char const end_code[2] = { 0x00, 0x00 };
		int len, got_frame;
		AVPacket pkt;
		av_init_packet(&pkt);

		char* data = new char[frameSize + sizeof(start_code)/* + sizeof(end_code)*/];
		pkt.size = frameSize + sizeof(start_code);// +5;// + sizeof(end_code);

		memcpy(data, start_code, sizeof(start_code));
		memcpy(data + sizeof(start_code), buffer, frameSize);
		//memcpy(data + sizeof(start_code)+frameSize, end_code, sizeof(end_code));
		pkt.data = (uint8_t*)data;

		AVFrame* yuvFrame = av_frame_alloc();
		if (!yuvFrame)
		{
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		len = avcodec_decode_video2(codecContext, yuvFrame, &got_frame, &pkt);
		//got_frame = 1;

		//envir() << "(" << nal_unit_type << ", " << frameSize << ", " << len << ", " << got_frame << ")\n";

		if (len < 0)
		{
			//std::cout << this << ": error decoding frame" << std::endl;
		}
		else if (len == 0)
		{
			std::cout << this << ": no frame could be decoded" << std::endl;
		}
		else if (got_frame == 1)
		{
			//std::cout << this << ": decoded frame (" << yuvFrame->width << ", " << yuvFrame->height << ")" << std::endl;


			AVFrame* frame;
			framePool.wait_and_pop(frame);

			if (!frame->data[0])
			{
				frame->width = yuvFrame->width;
				frame->height = yuvFrame->height;

				/* the image can be allocated by any means and av_image_alloc() is
				* just the most convenient way if av_malloc() is to be used */
				if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
					(AVPixelFormat)frame->format, 32) < 0)
				{
					fprintf(stderr, "Could not allocate raw picture buffer\n");
					exit(1);
				}
			}

			//x2y(yuvFrame, frame, codecContext);

			if (!img_convert_ctx)
			{
				img_convert_ctx = sws_getContext(
					yuvFrame->width, yuvFrame->height, (AVPixelFormat)yuvFrame->format,
					frame->width, frame->height, (AVPixelFormat)frame->format,
					SWS_BICUBIC, NULL, NULL, NULL);
			}

			int x = sws_scale(img_convert_ctx, yuvFrame->data,
				yuvFrame->linesize, 0, yuvFrame->height,
				frame->data, frame->linesize);

			// Flip image vertically
			for (int i = 0; i < 4; i++)
			{
				frame->data[i] += frame->linesize[i] * (frame->height - 1);
				frame->linesize[i] = -frame->linesize[i];
			}

			//std::cout << this << ": afterGettingFrame: " << frame << "\n";

			//printf("%ld.%06ld\n", presentationTime.tv_sec, presentationTime.tv_usec);

			av_free(yuvFrame);
			//av_frame_free(&yuvFrame);

			frameBuffer.push(frame);

			InvalidateRect(hWnd, NULL, TRUE);
		}

		//std::cout << this << "frame" << std::endl;
	}
	else
	{
		std::cout << this << " skipped frame" << std::endl;
	}

	// Then try getting the next frame:
	continuePlaying();
}

Boolean H264WindowSink::continuePlaying()
{
	fSource->getNextFrame(buffer, bufferSize,
		afterGettingFrame, this,
		onSourceClosure, this);

	return True;
}

void H264WindowSink::afterGettingFrame(void*clientData,
	unsigned frameSize,
	unsigned numTruncatedBytes,
	timeval presentationTime,
	unsigned durationInMicroseconds)
{
	H264WindowSink* sink = (H264WindowSink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

void H264WindowSink::windowLoop()
{
	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, H264WindowSink::MsgProc0, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"AlloPlayer", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	hWnd = CreateWindow(L"AlloPlayer", L"AlloPlayer H264WindowSink",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);

	hwndThisMap[hWnd] = this;

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

	UnregisterClass(L"AlloPlayer", wc.hInstance);
}

LRESULT WINAPI H264WindowSink::MsgProc0(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	H264WindowSink* _this = hwndThisMap[hWnd];
	return _this->MsgProc(hWnd, msg, wParam, lParam);
}

int H264WindowSink::x2y(AVFrame *srcFrame, AVFrame *dstFrame, AVCodecContext *c)
{
	char *err = NULL;
	if (img_convert_ctx == NULL)
	{
		// MUST BE IMPLMENTED
		int w = srcFrame->width;
		int h = srcFrame->width;
		img_convert_ctx = sws_getContext(w, h, (AVPixelFormat)srcFrame->format, w, h,
			c->pix_fmt, SWS_BICUBIC,
			NULL, NULL, NULL);
		if (img_convert_ctx == NULL)
		{
			sprintf(err, "Cannot initialize the conversion context!\n");
			return -1;
		}
	}
	if (srcFrame->linesize[0] > 0)
	{
		srcFrame->data[0] += srcFrame->linesize[0] * (c->height - 1);
		srcFrame->linesize[0] = -srcFrame->linesize[0];
	}
	return sws_scale(img_convert_ctx, srcFrame->data,
		srcFrame->linesize, 0, c->height,
		dstFrame->data, dstFrame->linesize);
}

LRESULT H264WindowSink::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:

		//std::cout << this << ": WM_PAINT" << std::endl;

		AVFrame* frame;

		frameBuffer.wait_and_pop(frame);

		//std::cout << this << ": WM_PAINT         : " << frame << "\n";

		int wWidth = frame->width;
		int wHeight = frame->height;

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

		//boost::thread threads[6];

		//for (int i = 0; i < 6; i++)
		//{




			/*BitBlt(hMemDC,
			cubeMapFacePositions[i].horizontal * wWidth,
			cubeMapFacePositions[i].vertical * wHeight,
			wWidth, wHeight, hFaceMemDC, 0, 0, SRCCOPY);*/
			/*StretchBlt(hMemDC,
			clientRect.right * cubeMapFacePositions[i].horizontal,
			clientRect.bottom * cubeMapFacePositions[i].vertical,
			clientRect.right / horizontalTilesCount, clientRect.bottom / verticalTilesCount,
			hFaceMemDC, 0, 0, wWidth, wHeight, SRCCOPY);*/

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

			int ret = avpicture_layout((AVPicture*)frame, (AVPixelFormat)frame->format,
				frame->width, frame->height, (unsigned char*)dwBytes, wWidth * wHeight * 4);


			//memcpy(dwBytes, frame->data[0], wWidth * wHeight * 4);

			StretchBlt(hMemDC,
				0,
				0,
				clientRect.right, clientRect.bottom,
				hFaceMemDC, 0, 0, wWidth, wHeight, SRCCOPY);

			DeleteObject(hFaceMemBitmap);
			DeleteDC(hFaceMemDC);

			//threads[i] = boost::thread(&StretchBltThreadRun, hMemDC, hDC, clientRect, wWidth, wHeight, i);
		//}

		/*for (int i = 0; i < 6; i++)
		{
			threads[i].join();
		}*/

		//StretchBlt(hDC, 0, 0, clientRect.right, clientRect.bottom,
		//hMemDC, 0, 0, wWidth * 4, wHeight * 3, SRCCOPY);

		//SetDIBits(hDC, hBitmap, 0, wHeight, cubemapFaces[0].pixels, &bmi, DIB_RGB_COLORS);



		BitBlt(hDC, 0, 0, clientRect.right, clientRect.bottom, hMemDC, 0, 0, SRCCOPY);

		DeleteObject(hMemBitmap);
		DeleteDC(hMemDC);
		EndPaint(hWnd, &ps);

		framePool.push(frame);

		return 0;
		
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}