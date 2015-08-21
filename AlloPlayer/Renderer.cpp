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
            return nullptr;
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
    
//    static GLuint* pixels = nullptr;
//    static GLuint mTextureID = 0;
//    
//    int width = mOmni.resolution(), height = mOmni.resolution();
//    
//    if (!pixels)
//    {
//        //Checkerboard pixels
//        const int CHECKERBOARD_PIXEL_COUNT = width * height;
//        pixels = new GLuint[CHECKERBOARD_PIXEL_COUNT];
//        
//        //Go through pixels
//        for( int i = 0; i < CHECKERBOARD_PIXEL_COUNT; ++i )
//        {
//            //Get the individual color components
//            GLubyte* colors = (GLubyte*)&pixels[ i ];
//            
//            //If the 5th bit of the x and y offsets of the pixel do not match
//            if( i / width & 16 ^ i % height & 16 )
//            {
//                //Set pixel to white
//                colors[ 0 ] = 0xFF;
//                colors[ 1 ] = 0x00;
//                colors[ 2 ] = 0x00;
//                colors[ 3 ] = 0xFF;
//            }
//            else
//            {
//                //Set pixel to red
//                colors[ 0 ] = 0x00;
//                colors[ 1 ] = 0x00;
//                colors[ 2 ] = 0xFF;
//                colors[ 3 ] = 0xFF;
//            }
//        }
//    }
//    
//    if (mTextureID == 0)
//    {
//        //Generate texture ID
//        glGenTextures( 1, &mTextureID );
//        
//        //Bind texture ID
//        glBindTexture( GL_TEXTURE_2D, mTextureID );
//        
//        //Generate texture
//        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
//        
//        //Set texture parameters
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//        
//        //Unbind texture
//        glBindTexture( GL_TEXTURE_2D, NULL );
//        
//        //Check for error
//        GLenum error = glGetError();
//        if( error != GL_NO_ERROR )
//        {
//            printf( "Error loading texture from %p pixels! %s\n", pixels, gluErrorString( error ) );
//            abort();
//        }
//    }

    int width = 512, height = 512;
    
    static al::Texture tex(width, height, al::Graphics::RGBA, al::Graphics::UBYTE);
    
    {
        // The default magnification filter is linear
        //tex.filterMag(Texture::NEAREST);
        
        // Get a pointer to the (client-side) pixel buffer.
        // When we make a read access to the pixels, they are flagged as dirty
        // and get sent to the GPU the next time the texture bound.
        unsigned char * pixels = tex.data<unsigned char>();
        
        // Loop through the pixels to generate an image
        int Nx = tex.width();
        int Ny = tex.height();
        for(int j=0; j<Ny; ++j)
        {
            for(int i=0; i<Nx; ++i)
            {
                int idx = j*Nx + i;
                int stride = tex.numComponents();
                if ((j / (height / 4)) % 2 ^ (i / (width / 4)) % 2)
                {
                    pixels[idx*stride + 0] = 255;
                    pixels[idx*stride + 1] = 0;
                    pixels[idx*stride + 2] = 0;
                }
                else
                {
                    pixels[idx*stride + 0] = 0;
                    pixels[idx*stride + 1] = 255;
                    pixels[idx*stride + 2] = 0;
                }
            }
        }
    }
    
    {
        StereoCubemap* cubemap;
        if (!cubemapBuffer.try_pop(cubemap))
        {
            if (cubemapBuffer.closed())
            {
                return;
            }
            else
            {
                cubemap = currentCubemap;
            }
        }
        
        currentCubemap = cubemap;
        
        // render cubemap
        if (cubemap && cubemap->getEyesCount() > eyeIndex)
        {
            Cubemap* eye = cubemap->getEye(eyeIndex);
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
//                
//                glUseProgram(0);
//                /*glDepthMask(GL_FALSE);
//                
//                // flip face
//                glPixelZoom(-1.0, 1.0);
//                glRasterPos2d(1.0, -1.0);
//                
//                // draw the background
//                glDrawPixels(face->getContent()->getWidth(),
//                             face->getContent()->getHeight(),
//                             GL_RGBA,
//                             GL_UNSIGNED_BYTE,
//                             (GLvoid*)face->getContent()->getPixels());
//                
//                glDepthMask(GL_TRUE);*/
//                
//                glUseProgram(0);
//                
//                //Initialize Projection Matrix
//                glMatrixMode( GL_PROJECTION );
//                glLoadIdentity();
//                glOrtho( 0.0, mOmni.resolution(), mOmni.resolution(), 0.0, 1.0, -1.0 );
//                
//                //Initialize Modelview Matrix
//                glMatrixMode( GL_MODELVIEW );
//                glLoadIdentity();
//                
//                //Initialize clear color
//                glClearColor( 0.f, 0.f, 0.f, 1.f );
//                
//                //Enable texturing
//                glEnable( GL_TEXTURE_2D );
//                
//                //Check for error
//                GLenum error = glGetError();
//                if( error != GL_NO_ERROR )
//                {
//                    printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
//                    abort();
//                }
//                
//                //Clear color buffer
//                glClear( GL_COLOR_BUFFER_BIT );
//                
//                //Remove any previous transformations
//                glLoadIdentity();
//                
//                //Set texture ID
//                glBindTexture( GL_TEXTURE_2D, mTextureID );
//                
//                glTexSubImage2D ( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
//                
//                //Render textured quad
//                glBegin( GL_QUADS );
//                glTexCoord2f( 0.0f, 0.0f ); glVertex2f( -mOmni.resolution(), -mOmni.resolution() );
//                glTexCoord2f( 1.0f, 0.0f ); glVertex2f( 0.0f, -mOmni.resolution() );
//                glTexCoord2f( 1.0f, 1.0f ); glVertex2f( 0.0f, 0.0f );
//                glTexCoord2f( 0.0f, 1.0f ); glVertex2f( -mOmni.resolution(), 0.0f );
//                glEnd();
//                
//                glDisable( GL_TEXTURE_2D );
                
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
                m.texCoord(0,1);
                m.texCoord(0,0);
                m.texCoord(1,1);
                m.texCoord(1,0);
                
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
        
        cubemapPool.push(cubemap);
    }

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
