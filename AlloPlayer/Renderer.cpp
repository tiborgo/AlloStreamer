#include "Renderer.hpp"

static const char* defaultVertShader = AL_STRINGIFY
(
    void main(void)
    {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    }
);

static const char* yuvGammaFragShader = AL_STRINGIFY
(
    uniform sampler2D yTexture;
    uniform sampler2D uTexture;
    uniform sampler2D vTexture;
 
    uniform float gamma_pow;
    uniform float gamma_min;
    uniform float gamma_max;
 
    void main(void)
    {
        float y = texture2D(yTexture, gl_TexCoord[0].st).r;
        float u = texture2D(uTexture, gl_TexCoord[0].st).r;
        float v = texture2D(vTexture, gl_TexCoord[0].st).r;
        
        // YUV -> RGB
        vec4 color;
        color.r = 1.164 * (y - 16.0/255.0)                             + 2.018 * (v - 128.0/255.0);
        color.g = 1.164 * (y - 16.0/255.0) - 0.813 * (u - 128.0/255.0) - 0.391 * (v - 128.0/255.0);
        color.b = 1.164 * (y - 16.0/255.0) + 1.596 * (u - 128.0/255.0);
        
        // Gamma
        color.r = pow(clamp(color.r, gamma_min, gamma_max), gamma_pow);
        color.g = pow(clamp(color.g, gamma_min, gamma_max), gamma_pow);
        color.b = pow(clamp(color.b, gamma_min, gamma_max), gamma_pow);
        gl_FragColor = color;
    }
);

Renderer::Renderer()
    :
    al::OmniApp("AlloPlayer", false, 2048), gammaMin(0.0f), gammaMax(1.0f), gammaPow(1.0f),
    forRotation(0, 0, 0), forAngle(M_PI*2.0), rotation(0, 0, 0), rotationSpeed(0.01)
{
    nav().smooth(0.8);
    
    for (int i = 0; i < 1; i++)
    {
        cubemapPool.push(nullptr);
    }
    
    for (int i = 0; i < StereoCubemap::MAX_EYES_COUNT * Cubemap::MAX_FACES_COUNT; i++)
    {
        textures.push_back(YUV420PTexture());
    }
}

Renderer::~Renderer()
{
}

bool Renderer::onCreate()
{
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    
    al::Shader vert, frag;
    vert.source(defaultVertShader, al::Shader::VERTEX).compile();
    vert.printLog();
    frag.source(yuvGammaFragShader, al::Shader::FRAGMENT).compile();
    frag.printLog();
    yuvGammaShader.attach(vert).attach(frag).link();
    yuvGammaShader.printLog();
    
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
                
                // Reorder faces so that they are displayed correctly in the AlloSphere
                int texI;
                if (i == 0)
                {
                    texI = 1;
                }
                else if (i == 1)
                {
                    texI = 0;
                }
                else if (i == 2)
                {
                    texI = 4;
                }
                else if (i == 3)
                {
                    texI = 5;
                }
                else if (i == 4)
                {
                    texI = 2;
                }
                else if (i == 5)
                {
                    texI = 3;
                }
                else
                {
                    texI = i;
                }
                YUV420PTexture& tex = textures[texI + j * Cubemap::MAX_FACES_COUNT];
                
                if (face)
                {
                    // create texture if not already created
                    if (!tex.yTexture)
                    {
                        tex.yTexture = new al::Texture(face->getContent()->getWidth(),
                                                       face->getContent()->getHeight(),
                                                       al::Graphics::LUMINANCE,
                                                       al::Graphics::UBYTE);
                        tex.uTexture = new al::Texture(face->getContent()->getWidth()/2,
                                                       face->getContent()->getHeight()/2,
                                                       al::Graphics::LUMINANCE,
                                                       al::Graphics::UBYTE);
                        tex.vTexture = new al::Texture(face->getContent()->getWidth()/2,
                                                       face->getContent()->getHeight()/2,
                                                       al::Graphics::LUMINANCE,
                                                       al::Graphics::UBYTE);
                        textures[texI + j * Cubemap::MAX_FACES_COUNT] = tex;
                        
                        // In case a face is mono use the same the texture for left and right.
                        // By doing so, image will become twice as bright in the AlloSphere.
                        if (j == 0 && cubemap->getEyesCount() > 1 && cubemap->getEye(1)->getFacesCount() <= i)
                        {
                            textures[texI + Cubemap::MAX_FACES_COUNT] = tex;
                        }
                    }
                    
                    tex.yTexture->bind();
                    glTexSubImage2D(tex.yTexture->target(), 0,
                                    0, 0,
                                    tex.yTexture->width(),
                                    tex.yTexture->height(),
                                    tex.yTexture->format(),
                                    tex.yTexture->type(),
                                    face->getContent()->getPixels());
                    tex.vTexture->bind();
                    glTexSubImage2D(tex.vTexture->target(), 0,
                                    0, 0,
                                    tex.vTexture->width(),
                                    tex.vTexture->height(),
                                    tex.vTexture->format(),
                                    tex.vTexture->type(),
                                    (char*)face->getContent()->getPixels() +
                                        face->getContent()->getWidth() * face->getContent()->getHeight());
                    tex.uTexture->bind();
                    glTexSubImage2D(tex.uTexture->target(), 0,
                                    0, 0,
                                    tex.uTexture->width(),
                                    tex.uTexture->height(),
                                    tex.uTexture->format(),
                                    tex.uTexture->type(),
                                    (char*)face->getContent()->getPixels() +
                                        face->getContent()->getWidth() * face->getContent()->getHeight() +
                                        (face->getContent()->getWidth()/2) * (face->getContent()->getHeight()/2));
                    tex.uTexture->unbind();
                    
                    if (onDisplayedCubemapFace) onDisplayedCubemapFace(this, i + j * Cubemap::MAX_FACES_COUNT);
                }
            }
        }
        
        cubemapPool.push(cubemap);
    }
    
    // Set uniforms
    {
        boost::mutex::scoped_lock(uniformsMutex);
        yuvGammaShader.begin();
        yuvGammaShader.uniform("gamma_min", gammaMin);
        yuvGammaShader.uniform("gamma_max", gammaMax);
        yuvGammaShader.uniform("gamma_pow", gammaPow);
        yuvGammaShader.end();
        mOmni.forRotation(forRotation);
        mOmni.forAngle(forAngle);
        mOmni.rotation(rotation);
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
    YUV420PTexture& tex = textures[faceIndex + eyeIndex * Cubemap::MAX_FACES_COUNT];
    
    // render cubemap
    if (tex.yTexture)
    {
        // Configure gamma to make backdrop more visible in the AlloSphere
        yuvGammaShader.begin();
        
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
        tex.yTexture->bind(0);
        tex.vTexture->bind(1);
        tex.uTexture->bind(2);
        
        yuvGammaShader.uniform("yTexture", 0);
        yuvGammaShader.uniform("uTexture", 1);
        yuvGammaShader.uniform("vTexture", 2);
        
        gl.draw(m);
        
        tex.yTexture->unbind(0);
        tex.vTexture->unbind(1);
        tex.uTexture->unbind(2);
        
        yuvGammaShader.end();
    }
}

void Renderer::onMessage(al::osc::Message& m)
{
    OmniApp::onMessage(m);
 
    m.resetStream();
    if (m.addressPattern() == "/ty")
    {
        float x;
        m >> x;
        setRotation(al::Vec3f(0, rotation[1]-x/rotationSpeed, 0));
    }
}

bool Renderer::onKeyDown(const al::Keyboard& k)
{
    return true;
}

void Renderer::setOnDisplayedFrame(const std::function<void (Renderer*)>& callback)
{
    onDisplayedFrame = callback;
}

void Renderer::setOnDisplayedCubemapFace(const std::function<void (Renderer*, int)>& callback)
{
    onDisplayedCubemapFace = callback;
}

void Renderer::setGammaMin(float gammaMin)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->gammaMin = gammaMin;
}

void Renderer::setGammaMax(float gammaMax)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->gammaMax = gammaMax;
}

void Renderer::setGammaPow(float gammaPow)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->gammaPow = gammaPow;
}

void Renderer::setFORRotation(const al::Vec3f& forRotation)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->forRotation = forRotation;
}

void Renderer::setFORAngle(float forAngle)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->forAngle = forAngle;
}

void Renderer::setRotation(const al::Vec3f& rotation)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->rotation = rotation;
}

void Renderer::setRotationSpeed(float speed)
{
    boost::mutex::scoped_lock(uniformsMutex);
    this->rotationSpeed = speed;
}

void Renderer::setCubemapSource(CubemapSource* cubemapSource)
{
    cubemapSource->setOnNextCubemap(boost::bind(&Renderer::onNextCubemap,
                                                this,
                                                _1,
                                                _2));
}

float Renderer::getGammaMin()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return gammaMin;
}

float Renderer::getGammaMax()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return gammaMax;
}

float Renderer::getGammaPow()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return gammaPow;
}

const al::Vec3f& Renderer::getFORRotation()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return forRotation;
}

float Renderer::getFORAngle()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return forAngle;
}

const al::Vec3f& Renderer::getRotation()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return rotation;
}

float Renderer::getRotationSpeed()
{
    boost::mutex::scoped_lock(uniformsMutex);
    return rotationSpeed;
}
