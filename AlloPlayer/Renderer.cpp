#include "Renderer.hpp"

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

Renderer::Renderer(CubemapSource* cubemapSource)
    :
    al::OmniApp("AlloPlayer", false, 2048), resizeCtx(nullptr), cubemapSource(cubemapSource),
    cubemap(nullptr), newCubemap(false)
{
    nav().smooth(0.8);
    
    // set up cube
    cube.color(1,1,1,1);
    cube.primitive(al::Graphics::TRIANGLES);
    addCube(cube);
    for (int i = 0; i < cube.vertices().size(); ++i) {
        float f = (float)i / cube.vertices().size();
        cube.color(al::Color(al::HSV(f, 1 - f, 1), 1));
    }
    cube.generateNormals();
    
    // set up sphere
    sphere.primitive(al::Graphics::TRIANGLES);
    addSphere(sphere, 1.0, 32, 32);
    for (int i = 0; i < sphere.vertices().size(); ++i) {
        float f = (float)i / sphere.vertices().size();
        sphere.color(al::Color(al::HSV(f, 1 - f, 1), 1));
    }
    sphere.generateNormals();
    
    // set up light
    light.ambient(al::Color(0.4, 0.4, 0.4, 1.0));
    light.pos(5, 5, 5);
    
    
    
    std::function<void (CubemapSource*, StereoCubemap*)> callback = boost::bind(&Renderer::onNextCubemap,
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
        boost::mutex::scoped_lock lock(nextCubemapMutex);
        result = OmniApp::onFrame();
        newCubemap = false;
    }
    if (onDisplayedFrame) onDisplayedFrame(this);
    return result;
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

void Renderer::onDraw(al::Graphics& gl)
{
    int faceIndex = mOmni.face();
    
    {
        
        
        // render cubemap
        if (cubemap && cubemap->getEyesCount() > 0)
        {
            Cubemap* eye = cubemap->getEye(0);
            if (eye->getFacesCount() > faceIndex)
            {
                CubemapFace* face = eye->getFace(faceIndex);
                
                glUseProgram(0);
                glDepthMask(GL_FALSE);
                
                // draw the background
                glDrawPixels(face->getContent()->getWidth(),
                             face->getContent()->getHeight(),
                             GL_RGB,
                             GL_UNSIGNED_BYTE,
                             (GLvoid*)face->getContent()->getPixels());
                
                glDepthMask(GL_TRUE);
                
                
                
                if(newCubemap)
                {
                    if (onDisplayedCubemapFace) onDisplayedCubemapFace(this, faceIndex);
                }
            }
        }
    }
    
    light();
    mShader.begin();
    mOmni.uniforms(mShader);
    
    gl.pushMatrix();
    
    //gl.draw(cube);
    
    gl.pushMatrix();
    // rotate over time:
    gl.rotate(now*30., 0.707, 0.707, 0.);
    gl.translate(1., 1., 1.);
    //gl.draw(sphere);
    gl.popMatrix();
    
    gl.popMatrix();
    
    mShader.end();
}


void Renderer::onAnimate(al_sec dt)
{
    pose = nav();
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
