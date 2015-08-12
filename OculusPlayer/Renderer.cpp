#include "Renderer.hpp"

// Include DirectX
#include "Win32_DirectXAppUtil.h" 

// Include the Oculus SDK
#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"
using namespace OVR;

Renderer::Renderer(CubemapSource* cubemapSource)
    :
	cubemapSource(cubemapSource), texture(nullptr)
{
	OculusInit();


    std::function<void (CubemapSource*, StereoCubemap*)> callback = boost::bind(&Renderer::onNextCubemap,
                                                                                this,
                                                                                _1,
                                                                                _2);
    cubemapSource->setOnNextCubemap(callback);

	for (int i = 0; i < 1; i++)
	{
		cubemapPool.push(nullptr);
	}

	if (SDL_Init(SDL_INIT_VIDEO/* | SDL_INIT_TIMER*/))
	{
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		abort();
	}

	/*screen = SDL_CreateWindow("Windowed Player",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		0);

	if (!screen) {
		fprintf(stderr, "SDL: could not open window - exiting\n");
		abort;
	}*/

	//Now create a window with title "Hello World" at 100, 100 on the screen with w:640 h:480 and show it
	window = SDL_CreateWindow("Hello World!", 100, 100, 500, 500, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	//Make sure creating our window went ok
	if (window == nullptr){
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		abort();
	}

	//Create a renderer that will draw to the window, -1 specifies that we want to load whichever
	//video driver supports the flags we're passing
	//Flags: SDL_RENDERER_ACCELERATED: We want to use hardware accelerated rendering
	//SDL_RENDERER_PRESENTVSYNC: We want the renderer's present function (update screen) to be
	//synchornized with the monitor's refresh rate
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		SDL_DestroyWindow(window);
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		abort();
	}

	//SDL 2.0 now uses textures to draw things but SDL_LoadBMP returns a surface
	//this lets us choose when to upload or remove textures from the GPU
	//std::string imagePath = getResourcePath("Lesson1") + "hello.bmp";
	/*bmp = SDL_LoadBMP(imagePath.c_str());
	if (bmp == nullptr){
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		abort();
	}*/

}

Renderer::~Renderer()
{
	cubemapBuffer.close();
	cubemapPool.close();
	renderThread.join();

	//Clean up our objects and quit
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();


	OculusRelease();
}

void Renderer::onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
	
	StereoCubemap* dummy;
	if (!cubemapPool.wait_and_pop(dummy))
	{
		return;
	}
	cubemapBuffer.push(cubemap);

	void *pixels[12];
	UINT w = cubemap->getEye(0)->getFace(0)->getContent()->getWidth(), h=cubemap->getEye(0)->getFace(0)->getContent()->getWidth();
	for (int e = 0; e < 2; e++){
		for (int i = 0; i < 6; i++){
			pixels[i + 6 * e] = cubemap->getEye(0)->getFace(i)->getContent()->getPixels();	//Draws left eye for both eyes
		}
	}
	
	scene->updateTextures(pixels, w, h);
	OculusRenderLoop();
}

void Renderer::setOnDisplayedFrame(std::function<void (Renderer*)>& callback)
{
    onDisplayedFrame = callback;
}

void Renderer::setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback)
{
    onDisplayedCubemapFace = callback;
}

void Renderer::OculusInit(){
	hinst = (HINSTANCE)GetModuleHandle(NULL);
	// Initializes LibOVR, and the Rift
	ovrResult result = ovr_Initialize(nullptr);
	VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");

	ovrResult actualHMD = ovrHmd_Create(0, &HMD);
	if (!OVR_SUCCESS(actualHMD)) result = ovrHmd_CreateDebug(ovrHmd_DK2, &HMD); // Use debug one, if no genuine Rift available
	VALIDATE(OVR_SUCCESS(result), "Oculus Rift not detected.");
	VALIDATE(HMD->ProductName[0] != '\0', "Rift detected, display not enabled.");

	// Setup Window and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	ovrSizei winSize = { HMD->Resolution.w / 2, HMD->Resolution.h / 2 };
	bool initialized = DIRECTX.InitWindowAndDevice(hinst, Recti(Vector2i(0), winSize), L"Oculus Room Tiny (DX11)");
	VALIDATE(initialized, "Unable to initialize window and D3D11 device.");

	ovrHmd_SetEnabledCaps(HMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

	// Start the sensor which informs of the Rift's pose and motion
	result = ovrHmd_ConfigureTracking(HMD, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection |
		ovrTrackingCap_Position, 0);
	VALIDATE(!OVR_SUCCESS(actualHMD) || OVR_SUCCESS(result), "Failed to configure tracking.");

	// Make the eye render buffers (caution if actual size < requested due to HW limits). 
	

	for (int eye = 0; eye < 2; eye++)
	{
		Sizei idealSize = ovrHmd_GetFovTextureSize(HMD, (ovrEyeType)eye, HMD->DefaultEyeFov[eye], 1.0f);
		pEyeRenderTexture[eye] = new OculusTexture(HMD, idealSize);
		pEyeDepthBuffer[eye] = new DepthBuffer(DIRECTX.Device, idealSize);
		eyeRenderViewport[eye].Pos = Vector2i(0, 0);
		eyeRenderViewport[eye].Size = idealSize;
	}

	// Create a mirror to see on the monitor.
	mirrorTexture = nullptr;
	D3D11_TEXTURE2D_DESC td = {};
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.Width = DIRECTX.WinSize.w;
	td.Height = DIRECTX.WinSize.h;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.SampleDesc.Count = 1;
	td.MipLevels = 1;
	ovrHmd_CreateMirrorTextureD3D11(HMD, DIRECTX.Device, &td, &mirrorTexture);

	// Create the room model
	scene=new Scene();

	// Create camera
	mainCam=Camera(Vector3f(0.0f, 0.0f, 0.0f), Matrix4f::RotationY(0.0f));

	// Setup VR components, filling out description
	
	eyeRenderDesc[0] = ovrHmd_GetRenderDesc(HMD, ovrEye_Left, HMD->DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovrHmd_GetRenderDesc(HMD, ovrEye_Right, HMD->DefaultEyeFov[1]);

	bool isVisible = true;

}

void Renderer::OculusRenderLoop(){
	// Keyboard inputs to adjust player orientation, unaffected by speed
	static float Yaw = 3.141f;

	if (DIRECTX.Key[VK_LEFT])  mainCam.Rot = Matrix4f::RotationY(Yaw += 0.02f);
	if (DIRECTX.Key[VK_RIGHT]) mainCam.Rot = Matrix4f::RotationY(Yaw -= 0.02f);

	// Keyboard inputs to adjust player position
	if (DIRECTX.Key['W'] || DIRECTX.Key[VK_UP])   mainCam.Pos += mainCam.Rot.Transform(Vector3f(0, 0, -0.05f));
	if (DIRECTX.Key['S'] || DIRECTX.Key[VK_DOWN]) mainCam.Pos += mainCam.Rot.Transform(Vector3f(0, 0, +0.05f));
	if (DIRECTX.Key['D'])                         mainCam.Pos += mainCam.Rot.Transform(Vector3f(+0.05f, 0, 0));
	if (DIRECTX.Key['A'])                         mainCam.Pos += mainCam.Rot.Transform(Vector3f(-0.05f, 0, 0));
	mainCam.Pos.y = ovrHmd_GetFloat(HMD, OVR_KEY_EYE_HEIGHT, 0);

	// Animate the cube
	static float cubeClock = 0;
	//roomScene.Models[0]->Pos = Vector3f(9 * sin(cubeClock), 3, 9 * cos(cubeClock+=0.015f));

	// Get both eye poses simultaneously, with IPD offset already included. 
	ovrPosef         EyeRenderPose[2];
	ovrVector3f      HmdToEyeViewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset,
		eyeRenderDesc[1].HmdToEyeViewOffset };
	ovrFrameTiming   ftiming = ovrHmd_GetFrameTiming(HMD, 0);
	ovrTrackingState hmdState = ovrHmd_GetTrackingState(HMD, ftiming.DisplayMidpointSeconds);
	ovr_CalcEyePoses(hmdState.HeadPose.ThePose, HmdToEyeViewOffset, EyeRenderPose);

	
	
		// Render Scene to Eye Buffers
		for (int eye = 0; eye < 2; eye++)
		{
			// Increment to use next texture, just before writing
			pEyeRenderTexture[eye]->AdvanceToNextTexture();

			// Clear and set up rendertarget
			int texIndex = pEyeRenderTexture[eye]->TextureSet->CurrentIndex;
			DIRECTX.SetAndClearRenderTarget(pEyeRenderTexture[eye]->TexRtv[texIndex], pEyeDepthBuffer[eye]);
			DIRECTX.SetViewport(Recti(eyeRenderViewport[eye]));

			// Get view and projection matrices for the Rift camera
			Camera finalCam(mainCam.Pos + mainCam.Rot.Transform(EyeRenderPose[eye].Position),
				mainCam.Rot * Matrix4f(EyeRenderPose[eye].Orientation));
			Matrix4f view = finalCam.GetViewMatrix();
			Matrix4f proj = ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.2f, 1000.0f, ovrProjection_RightHanded);

			// Render the scene
			scene->Render(proj*view, 1, 1, 1, 1, true);
		}
	

	// Initialize our single full screen Fov layer.
	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = 0;

	for (int eye = 0; eye < 2; eye++)
	{
		ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureSet;
		ld.Viewport[eye] = eyeRenderViewport[eye];
		ld.Fov[eye] = HMD->DefaultEyeFov[eye];
		ld.RenderPose[eye] = EyeRenderPose[eye];
	}

	// Set up positional data.
	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
	viewScaleDesc.HmdToEyeViewOffset[0] = HmdToEyeViewOffset[0];
	viewScaleDesc.HmdToEyeViewOffset[1] = HmdToEyeViewOffset[1];

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovrHmd_SubmitFrame(HMD, 0, &viewScaleDesc, &layers, 1);
	

	// Render mirror
	ovrD3D11Texture* tex = (ovrD3D11Texture*)mirrorTexture;
	DIRECTX.Context->CopyResource(DIRECTX.BackBuffer, tex->D3D11.pTexture);
	DIRECTX.SwapChain->Present(0, 0);
}

void Renderer::OculusRelease(){
	// Release 
	ovrHmd_DestroyMirrorTexture(HMD, mirrorTexture);
	pEyeRenderTexture[0]->Release(HMD);
	pEyeRenderTexture[1]->Release(HMD);
	ovrHmd_Destroy(HMD);
	ovr_Shutdown();
	DIRECTX.ReleaseWindow(hinst);
}

void Renderer::start()
{

	

	//--------------------------------------------------------------------
	renderThread = boost::thread(boost::bind(&Renderer::renderLoop, this));

	SDL_Event evt;
	while (true)
	{
		SDL_WaitEvent(&evt);
		if (evt.type == SDL_QUIT)
		{
			return;
		}
	}
}

void Renderer::renderLoop()
{
	static int counter = 0;

	while (true)
	{
		StereoCubemap* cubemap;

		if (!cubemapBuffer.wait_and_pop(cubemap))
		{
			return;
		}

		Frame* content = cubemap->getEye(0)->getFace(0)->getContent();

		if (!texture)
		{
			//To use a hardware accelerated texture for rendering we can create one from
			//the surface we loaded
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, content->getWidth(), content->getHeight());
			//We no longer need the surface
			//SDL_FreeSurface(bmp);
			if (texture == nullptr){
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
				SDL_Quit();
				abort();
			}
		}

		// Show cubemap
		//A sleepy rendering loop, wait for 3 seconds and render and present the screen each time
		//for (int i = 0; i < 3; ++i){
		
		if (counter % 5 == 0)
		{
			void* pixels;
			int   pitch;

			if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0)
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
				SDL_Quit();
				abort();
			}

			
			memcpy(pixels, content->getPixels(), content->getHeight() * content->getWidth() * 4);

			SDL_UnlockTexture(texture);


			//First clear the renderer
			SDL_RenderClear(renderer);
			//Draw the texture
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			//Update the screen
			SDL_RenderPresent(renderer);
			//Take a quick break after all that hard work
			//SDL_Delay(1000);

			//}

			if (onDisplayedCubemapFace) onDisplayedCubemapFace(this, 0);
			if (onDisplayedFrame) onDisplayedFrame(this);
		}


		

		StereoCubemap::destroy(cubemap);
		cubemapPool.push(nullptr);
		counter++;
	}
}