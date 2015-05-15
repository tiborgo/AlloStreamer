#include <alloutil/al_OmniApp.hpp>
#include <SOIL.h>

#include "H264RawPixelsSink.h"

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

struct DynamicCubemapBackgroundApp : al::OmniApp {
    al::Mesh cube, sphere;
    al::Light light;
    al_sec now;
    std::vector<H264RawPixelsSink*> sinks;
    boost::mutex sinkMutex;
    
    int width, height;
    unsigned char* cubemapFaces[6];

  DynamicCubemapBackgroundApp() {
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
      
      // load cubemap background
      std::vector<const GLchar*> faces;
      
      // back
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/right.jpg");
      // front
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/left.jpg");
      // top
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/top.jpg");
      // bottom
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/bottom.jpg");
      // left
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/back.jpg");
      // right
      faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox_small/front.jpg");
      
      for (GLuint i = 0; i < faces.size(); i++)
      {
          cubemapFaces[i] = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
      }
  }

  virtual ~DynamicCubemapBackgroundApp() {}

  bool onCreate() {
      
      std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
      return OmniApp::onCreate();
  }
    
    bool onFrame() {
        now = al::MainLoop::now();
        return OmniApp::onFrame();
    }

  void onDraw(al::Graphics& gl){
      
      int face = mOmni.face();
      
      {
          // get next frame
          boost::mutex::scoped_lock lock(sinkMutex);
          
          if (sinks.size() > face)
          {
              Frame* frame = sinks[face]->getNextFrame();
              
              glUseProgram(0);
              glDepthMask(GL_FALSE);
              
              // draw the background
              glDrawPixels(width,
                           height,
                           GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           (GLvoid*)frame);
              
              glDepthMask(GL_TRUE);
          }
      }

      

      light();
      mShader.begin();
      mOmni.uniforms(mShader);

      gl.pushMatrix();

      gl.draw(cube);
      
      gl.pushMatrix();
      // rotate over time:
      gl.rotate(now*30., 0.707, 0.707, 0.);
      gl.translate(5., 5., 5.);
      gl.draw(sphere);
      gl.popMatrix();
      
      gl.popMatrix();

      mShader.end();
  }


  virtual void onAnimate(al_sec dt) {
    pose = nav();
  }

  virtual void onMessage(al::osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const al::Keyboard& k) { return true; }
    
    void addSink(H264RawPixelsSink* sink)
    {
        boost::mutex::scoped_lock lock(sinkMutex);
        sinks.push_back(sink);
    }
};

int mainDynamicCubemapBackgroundApp(int argc, char* argv[]) {
  DynamicCubemapBackgroundApp().start();
  return 0;
}
