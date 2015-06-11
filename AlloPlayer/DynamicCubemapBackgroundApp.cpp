#include "DynamicCubemapBackgroundApp.hpp"

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

DynamicCubemapBackgroundApp::DynamicCubemapBackgroundApp(CubemapSource* cubemapSource)
    : al::OmniApp("AlloPlayer", false, 2048), cubemapSource(cubemapSource), cubemap(nullptr)
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
    
    resizeCtx = nullptr;
}

DynamicCubemapBackgroundApp::~DynamicCubemapBackgroundApp()
{
}

bool DynamicCubemapBackgroundApp::onCreate()
{
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    return OmniApp::onCreate();
}

bool DynamicCubemapBackgroundApp::onFrame()
{
    now = al::MainLoop::now();
    
    StereoCubemap* nextCubemap = cubemapSource->tryGetNextCubemap(mOmni.resolution(), AV_PIX_FMT_RGB24);
    if (nextCubemap)
    {
        if (cubemap)
        {
            StereoCubemap::destroy(cubemap);
        }
        cubemap = nextCubemap;
        newCubemap = true;
    }
    else
    {
        newCubemap = false;
    }
    
    //std::cout << "FPS: " << FPS::fps() << std::endl;
    bool result = OmniApp::onFrame();
    stats.displayedFrame();
    return result;
}

void DynamicCubemapBackgroundApp::onDraw(al::Graphics& gl)
{
    int faceIndex = mOmni.face();
    
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
            glDrawPixels(face->getWidth(),
                         face->getHeight(),
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         (GLvoid*)face->getPixels());
            
            glDepthMask(GL_TRUE);
            
            
            
            if(newCubemap)
            {
                stats.displayedCubemapFace(faceIndex);
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


void DynamicCubemapBackgroundApp::onAnimate(al_sec dt)
{
    pose = nav();
}

void DynamicCubemapBackgroundApp::onMessage(al::osc::Message& m)
{
    OmniApp::onMessage(m);
}

bool DynamicCubemapBackgroundApp::onKeyDown(const al::Keyboard& k)
{
    return true;
}
