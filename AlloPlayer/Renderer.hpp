#pragma once

#include <alloutil/al_OmniApp.hpp>
#include <boost/thread.hpp>
#include "AlloShared/concurrent_queue.h"
#include "AlloReceiver/AlloReceiver.h"

class Renderer : public al::OmniApp
{
public:
    Renderer(CubemapSource* cubemapSource);
    
    virtual ~Renderer();
    
    bool onCreate();
    bool onFrame();
    void onDraw(al::Graphics& gl);
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
    al_sec                           now;
    CubemapSource*                   cubemapSource;
};
