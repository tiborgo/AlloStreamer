#pragma once

#include <alloutil/al_OmniApp.hpp>
#include <boost/thread.hpp>
#include "AlloShared/ConcurrentQueue.hpp"
#include "AlloReceiver/AlloReceiver.h"

class Renderer : public al::OmniApp
{
public:
    Renderer();
    
    virtual ~Renderer();
    
    bool onCreate();
    bool onFrame();
    void onDraw(al::Graphics& gl);
    virtual void onMessage(al::osc::Message& m);
    virtual bool onKeyDown(const al::Keyboard& k);
    StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap);
    
    void setOnDisplayedFrame(const std::function<void (Renderer*)>& callback);
    void setOnDisplayedCubemapFace(const std::function<void (Renderer*, int)>& callback);
    
    void setGammaMin(float gammaMin);
    void setGammaMax(float gammaMax);
    void setGammaPow(float gammaPow);
    void setFORRotation(const al::Vec3f& forRotation);
    void setFORAngle(float forAngle);
    void setRotation(const al::Vec3f& rotation);
    void setRotationSpeed(float speed);
    void setForceMono(bool forceMono);
    void setCubemapSource(CubemapSource* source);
    
    float                            getGammaMin();
    float                            getGammaMax();
    float                            getGammaPow();
    const al::Vec3f&                 getFORRotation();
    float                            getFORAngle();
    const al::Vec3f&                 getRotation();
    float                            getRotationSpeed();
    std::vector<std::pair<int, int>> getFaceResolutions();
    bool                             getForceMono();
    
protected:
    std::function<void (Renderer*)> onDisplayedFrame;
    std::function<void (Renderer*, int)> onDisplayedCubemapFace;
    
private:
    struct YUV420PTexture
    {
        al::Texture* yTexture;
        al::Texture* uTexture;
        al::Texture* vTexture;
        YUV420PTexture() : yTexture(nullptr), uTexture(nullptr), vTexture(nullptr) {}
    };
    
    ConcurrentQueue<StereoCubemap*> cubemapBuffer;
    ConcurrentQueue<StereoCubemap*> cubemapPool;
    std::vector<YUV420PTexture>      textures;
    al_sec                           now;
    al::ShaderProgram                yuvGammaShader;
    boost::mutex                     uniformsMutex;
    float                            gammaMin;
    float                            gammaMax;
    float                            gammaPow;
    al::Vec3f                        forRotation;
    float                            forAngle;
    al::Vec3f                        rotation;
    float                            rotationSpeed;
    bool                             forceMono;
};
