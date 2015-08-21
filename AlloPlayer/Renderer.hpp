#pragma once

#include <alloutil/al_OmniApp.hpp>
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/frame.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
    #include <x264.h>
}
#include <boost/thread.hpp>
#include "AlloShared/concurrent_queue.h"
#include "AlloReceiver/AlloReceiver.h"

class Renderer : public al::OmniApp
{
public:
    al::Mesh cube, sphere;
    al::Light light;
    al_sec now;
    std::vector<AVFrame*> currentFrames;
    SwsContext* resizeCtx;
    CubemapSource* cubemapSource;

    Renderer(CubemapSource* cubemapSource);
    
    virtual ~Renderer();
    bool onCreate();
    bool onFrame();
    void onDraw(al::Graphics& gl);
    virtual void onAnimate(al_sec dt);
    virtual void onMessage(al::osc::Message& m);
    virtual bool onKeyDown(const al::Keyboard& k);
    StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap);
    
    void setOnDisplayedFrame(std::function<void (Renderer*)>& callback);
    void setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback);
    
protected:
    std::function<void (Renderer*)> onDisplayedFrame;
    std::function<void (Renderer*, int)> onDisplayedCubemapFace;
    
private:
    concurrent_queue<StereoCubemap*> cubemapBuffer;
    concurrent_queue<StereoCubemap*> cubemapPool;
    std::vector<al::Texture*>        textures;
};
