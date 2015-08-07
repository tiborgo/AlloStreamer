#include "Renderer.hpp"

Renderer::Renderer(CubemapSource* cubemapSource)
    :
    cubemapSource(cubemapSource)
{
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
	window = SDL_CreateWindow("Hello World!", 100, 100, 1024, 1024, SDL_WINDOW_SHOWN);
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

	//To use a hardware accelerated texture for rendering we can create one from
	//the surface we loaded
	//SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, 512, 512);
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
}

void Renderer::onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
	StereoCubemap* dummy;
	if (!cubemapPool.wait_and_pop(dummy))
	{
		return;
	}
	cubemapBuffer.push(cubemap);
}

void Renderer::setOnDisplayedFrame(std::function<void (Renderer*)>& callback)
{
    onDisplayedFrame = callback;
}

void Renderer::setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback)
{
    onDisplayedCubemapFace = callback;
}

void Renderer::start()
{
	renderThread = boost::thread(boost::bind(&Renderer::renderLoop, this));
}

void Renderer::renderLoop()
{
	while (true)
	{
		StereoCubemap* cubemap;

		if (!cubemapBuffer.wait_and_pop(cubemap))
		{
			return;
		}

		// Show cubemap
		//A sleepy rendering loop, wait for 3 seconds and render and present the screen each time
		//for (int i = 0; i < 3; ++i){
		
		void* pixels;
		int   pitch;

		if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
			SDL_Quit();
			abort();
		}

		Frame* content = cubemap->getEye(0)->getFace(0)->getContent();
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

		StereoCubemap::destroy(cubemap);
		cubemapPool.push(nullptr);
	}
}