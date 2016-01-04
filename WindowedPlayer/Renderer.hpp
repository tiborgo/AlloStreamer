#pragma once


#include <SDL.h>
#undef main
#include <boost/thread.hpp>
#include "AlloShared/ConcurrentQueue.hpp"
#include "AlloReceiver/AlloReceiver.h"

class Renderer
{
public:
    Renderer(CubemapSource* cubemapSource);
    virtual ~Renderer();
    
	void start();

    void setOnDisplayedFrame(const std::function<void (Renderer*)>& callback);
    void setOnDisplayedCubemapFace(const std::function<void (Renderer*, int)>& callback);
    
protected:
    std::function<void (Renderer*)> onDisplayedFrame;
    std::function<void (Renderer*, int)> onDisplayedCubemapFace;

private:
	StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap);
	void renderLoop();
	void createTextures(StereoCubemap* cubemap);

	boost::thread                    renderThread;
	CubemapSource*                   cubemapSource;
	ConcurrentQueue<StereoCubemap*> cubemapBuffer;
	ConcurrentQueue<StereoCubemap*> cubemapPool;
	SDL_Window*                      window;
	SDL_Renderer*                    renderer;
	std::vector<SDL_Texture*>        textures;
};
