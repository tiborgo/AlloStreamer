#include "Renderer.hpp"

Renderer::Renderer(CubemapSource* cubemapSource)
    :
    al::OmniApp("AlloPlayer", false, 2048), resizeCtx(nullptr), cubemapSource(cubemapSource), newCubemap(false), currentCubemap(nullptr)
{
    nav().smooth(0.8);
    
    for (int i = 0; i < 1; i++)
    {
        cubemapPool.push(nullptr);
    }
    
    std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)> callback = boost::bind(&Renderer::onNextCubemap,
                                                                                          this,
                                                                                          _1,
                                                                                          _2);
    cubemapSource->setOnNextCubemap(callback);
}

Renderer::~Renderer()
{
}

bool Renderer::onCreate()
{
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    return OmniApp::onCreate();
}

bool Renderer::onFrame()
{
    now = al::MainLoop::now();
    
    bool result;
    {
        result = OmniApp::onFrame();
        newCubemap = false;
    }
    if (onDisplayedFrame) onDisplayedFrame(this);
    return result;
}

StereoCubemap* Renderer::onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
    StereoCubemap* oldCubemap;
    if (!cubemapPool.try_pop(oldCubemap))
    {
        if (cubemapPool.closed())
        {
            abort();
        }
        else
        {
            return cubemap;
        }
    }
    
    cubemapBuffer.push(cubemap);
    return oldCubemap;
}

void Renderer::onDraw(al::Graphics& gl)
{
    int faceIndex = mOmni.face();
    int eyeIndex = (mOmni.eye() <= 0.0f) ? 0 : 1;

    {
        {
            StereoCubemap* cubemap;
            if (cubemapBuffer.try_pop(cubemap))
            {
                cubemapPool.push(currentCubemap);
                currentCubemap = cubemap;
            }
        }
        
        // render cubemap
        if (currentCubemap && currentCubemap->getEyesCount() > eyeIndex)
        {
            Cubemap* eye = currentCubemap->getEye(eyeIndex);
            if (eye->getFacesCount() > faceIndex)
            {
                // Choose right face for flipping
                CubemapFace* face;
                if (faceIndex == 0)
                {
                    face = eye->getFace(1);
                }
                else if (faceIndex == 1)
                {
                    face = eye->getFace(0);
                }
                else
                {
                    face = eye->getFace(faceIndex);
                }
                
                static al::Texture tex(face->getContent()->getWidth(),
                                       face->getContent()->getHeight(),
                                       al::Graphics::RGBA,
                                       al::Graphics::UBYTE);
                
                unsigned char * pixels = tex.data<unsigned char>();
                memcpy(pixels,
                       face->getContent()->getPixels(),
                       face->getContent()->getWidth() * face->getContent()->getHeight() * 4);
                
                al::ShaderProgram::use(0);
                
                // Borrow a temporary Mesh from Graphics
                al::Mesh& m = gl.mesh();
                
                m.reset();
                
                // Generate geometry
                m.primitive(al::Graphics::TRIANGLE_STRIP);
                m.vertex(-1,  1);
                m.vertex(-1, -1);
                m.vertex( 1,  1);
                m.vertex( 1, -1);
                
                // Add texture coordinates
                m.texCoord(1,1);
                m.texCoord(1,0);
                m.texCoord(0,1);
                m.texCoord(0,0);
                
                // We must tell the GPU to use the texture when rendering primitives
                tex.bind();
                gl.draw(m);
                tex.unbind();
                
                if(newCubemap)
                {
                    if (onDisplayedCubemapFace) onDisplayedCubemapFace(this, faceIndex + eyeIndex * Cubemap::MAX_FACES_COUNT);
                }
            }
        }
    }

}


void Renderer::onAnimate(al_sec dt)
{
}

void Renderer::onMessage(al::osc::Message& m)
{
    OmniApp::onMessage(m);
}

bool Renderer::onKeyDown(const al::Keyboard& k)
{
    return true;
}

void Renderer::setOnDisplayedFrame(std::function<void (Renderer*)>& callback)
{
    onDisplayedFrame = callback;
}

void Renderer::setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback)
{
    onDisplayedCubemapFace = callback;
}
