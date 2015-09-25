#include "Renderer.hpp"

Renderer::Renderer(CubemapSource* cubemapSource)
    :
    al::OmniApp("AlloPlayer", false, 2048), cubemapSource(cubemapSource)
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
    
    for (int i = 0; i < StereoCubemap::MAX_EYES_COUNT * Cubemap::MAX_FACES_COUNT; i++)
    {
        textures.push_back(nullptr);
    }
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
    
    StereoCubemap* cubemap;
    if (cubemapBuffer.try_pop(cubemap))
    {
        for (int j = 0; j < cubemap->getEyesCount(); j++)
        {
            Cubemap* eye = cubemap->getEye(j);
            for (int i = 0; i < eye->getFacesCount(); i++)
            {
                CubemapFace* face = eye->getFace(i);
                
                // The whole cubemap needs to be flipped in the AlloSphere
                // Therefore swap left and right face
                int texI = i;
                if (i == 0)
                {
                    texI = 1;
                }
                else if (i == 1)
                {
                    texI = 0;
                }
                al::Texture* tex = textures[texI + j * Cubemap::MAX_FACES_COUNT];
                
                if (face)
                {
                    // create texture if not already created
                    if (!tex)
                    {
                        tex = new al::Texture(face->getContent()->getWidth(),
                                              face->getContent()->getHeight(),
                                              al::Graphics::RGBA,
                                              al::Graphics::UBYTE);
                        textures[texI + j * Cubemap::MAX_FACES_COUNT] = tex;
                    }
                
                    void* pixels = tex->data<void>();
                    memcpy(pixels,
                           face->getContent()->getPixels(),
                           face->getContent()->getWidth() * face->getContent()->getHeight() * 4);
                    
                    if (onDisplayedCubemapFace) onDisplayedCubemapFace(this, i + j * Cubemap::MAX_FACES_COUNT);
                }
            }
        }
        cubemapPool.push(cubemap);
    }
    
    bool result = OmniApp::onFrame();
    
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
    al::Texture* tex = textures[faceIndex + eyeIndex * Cubemap::MAX_FACES_COUNT];
    
    // render cubemap
    if (tex)
    {
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
        
        // Add texture coordinates and flip cubemap
        m.texCoord(1,1);
        m.texCoord(1,0);
        m.texCoord(0,1);
        m.texCoord(0,0);
        
        // We must tell the GPU to use the texture when rendering primitives
        tex->bind();
        gl.draw(m);
        tex->unbind();
    }
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
