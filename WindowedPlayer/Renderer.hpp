#pragma once

/*extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/frame.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
}*/
#include <SDL.h>
#undef main
//#include <SDL_thread.h>
#include <boost/thread.hpp>
#include "AlloShared/concurrent_queue.h"
#include "AlloReceiver/AlloReceiver.h"

class Renderer
{
public:
    Renderer(CubemapSource* cubemapSource);
    virtual ~Renderer();
    
	void start();

    void setOnDisplayedFrame(std::function<void (Renderer*)>& callback);
    void setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback);
    
protected:
    std::function<void (Renderer*)> onDisplayedFrame;
    std::function<void (Renderer*, int)> onDisplayedCubemapFace;

private:
	void onNextCubemap(CubemapSource* source, StereoCubemap* cubemap);
	void renderLoop();

	boost::thread                    renderThread;
	CubemapSource*                   cubemapSource;
	concurrent_queue<StereoCubemap*> cubemapBuffer;
	concurrent_queue<StereoCubemap*> cubemapPool;
	SDL_Window*                      window;
	SDL_Renderer*                    renderer;
	SDL_Surface*                     bmp;
	std::vector<SDL_Texture*>        textures;
};
