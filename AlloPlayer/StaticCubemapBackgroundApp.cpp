#include <alloutil/al_OmniApp.hpp>
#include <SOIL.h>

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

struct StaticCubemapBackgroundApp : al::OmniApp {
    al::Mesh cube, sphere;
    al::Light light;
    al_sec now;
    
    int width, height;
    unsigned char* cubemapFaces[6];

  StaticCubemapBackgroundApp() {
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

  virtual ~StaticCubemapBackgroundApp() {}

  bool onCreate() {
      
      std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
      return OmniApp::onCreate();
  }
    
    bool onFrame() {
        now = al::MainLoop::now();
        return OmniApp::onFrame();
    }

  void onDraw(al::Graphics& gl){

      glUseProgram(0);
      glDepthMask(GL_FALSE);
      glDrawPixels(width,
                   height,
                   GL_RGB,
                   GL_UNSIGNED_BYTE,
                   cubemapFaces[mOmni.face()]);
      glDepthMask(GL_TRUE);

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
};

int mainStaticCubemapBackgroundApp(int argc, char* argv[]) {
  StaticCubemapBackgroundApp().start();
  return 0;
}
