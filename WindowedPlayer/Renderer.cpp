#include "Renderer.hpp"

Renderer::Renderer(CubemapSource* cubemapSource)
    :
    resizeCtx(nullptr), cubemapSource(cubemapSource),
    cubemap(nullptr), newCubemap(false)
{
    std::function<void (CubemapSource*, StereoCubemap*)> callback = boost::bind(&Renderer::onNextCubemap,
                                                                                this,
                                                                                _1,
                                                                                _2);
    cubemapSource->setOnNextCubemap(callback);
}

Renderer::~Renderer()
{
}

void Renderer::onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
    boost::mutex::scoped_lock lock(nextCubemapMutex);
    if (this->cubemap)
    {
        StereoCubemap::destroy(this->cubemap);
    }
    this->cubemap = cubemap;
    newCubemap = true;
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

}