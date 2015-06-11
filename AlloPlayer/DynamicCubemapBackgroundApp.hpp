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
#include "AlloReceiver/AlloReceiver.h"

struct DynamicCubemapBackgroundApp : al::OmniApp
{
    al::Mesh cube, sphere;
    al::Light light;
    al_sec now;
    std::vector<AVFrame*> currentFrames;
    SwsContext* resizeCtx;
    CubemapSource* cubemapSource;
    StereoCubemap* cubemap;
    bool newCubemap;

    DynamicCubemapBackgroundApp(CubemapSource* cubemapSource);
    
    virtual ~DynamicCubemapBackgroundApp();
    bool onCreate();
    bool onFrame();
    void onDraw(al::Graphics& gl);
    virtual void onAnimate(al_sec dt);
    virtual void onMessage(al::osc::Message& m);
    virtual bool onKeyDown(const al::Keyboard& k);
};
