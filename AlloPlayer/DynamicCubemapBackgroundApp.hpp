#pragma once

#include <alloutil/al_OmniApp.hpp>
#include <SOIL.h>

#include "H264RawPixelsSink.h"

struct DynamicCubemapBackgroundApp : al::OmniApp
{
    al::Mesh cube, sphere;
    al::Light light;
    al_sec now;
    std::vector<H264RawPixelsSink*> sinks;
    std::vector<AVFrame*> currentFrames;
    boost::mutex sinkMutex;
    SwsContext* resizeCtx;

    DynamicCubemapBackgroundApp();
    
    virtual ~DynamicCubemapBackgroundApp();

    bool onCreate();
    
    bool onFrame();

    void onDraw(al::Graphics& gl);

    virtual void onAnimate(al_sec dt);

    virtual void onMessage(al::osc::Message& m);

    virtual bool onKeyDown(const al::Keyboard& k);
    
    void addSink(H264RawPixelsSink* sink);
};

int mainDynamicCubemapBackgroundApp(int argc, char* argv[]);
