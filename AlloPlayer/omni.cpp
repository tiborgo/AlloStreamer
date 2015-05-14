#include "alloutil/al_OmniApp.hpp"
#include "allocore/al_Allocore.hpp"
#include "alloutil/al_CubeMapFBO.hpp"
using namespace al;

#include <SOIL.h>

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

struct Scene : Drawable {

    Mesh mesh, grid;
    Lens lens;

    Scene() {
        double world_radius = 50;

        lens.near(1).far(world_radius);

        // set up mesh:
        mesh.primitive(Graphics::TRIANGLES);
        double tri_size = 2;
        int count = 4000;
        for (int i=0; i<count; i++) {
            double x = rnd::uniformS(world_radius);
            double y = rnd::uniformS(world_radius);
            double z = rnd::uniformS(world_radius);
            for (int v=0; v<3; v++) {
                mesh.color(HSV(float(i)/count, v!=2, 1));
                mesh.vertex(x+rnd::uniformS(tri_size), y+rnd::uniformS(tri_size), z+rnd::uniformS(tri_size));
            }
        }

        // set up grid:
        grid.primitive(Graphics::LINES);
        double stepsize = 1./2;
        for (double x=-1; x<=1; x+= stepsize) {
        for (double y=-1; y<=1; y+= stepsize) {
            grid.vertex(x, y, 1);
            grid.vertex(x, y, -1);
        }}
        for (double x=-1; x<=1; x+= stepsize) {
        for (double z=-1; z<=1; z+= stepsize) {
            grid.vertex(x, 1, z);
            grid.vertex(x, -1, z);
        }}
        for (double y=-1; y<=1; y+= stepsize) {
        for (double z=-1; z<=1; z+= stepsize) {
            grid.vertex(1, y, z);
            grid.vertex(-1, y, z);
        }}
        grid.scale(world_radius);
    }

    void onDraw(Graphics& gl){
        gl.fog(lens.far(), lens.far()/2, Color(0, 0, 0));
        gl.depthTesting(1);
        gl.draw(grid);
        gl.draw(mesh);
    }
};

struct MyApp : OmniApp {
    Graphics gl;
    Mesh cube;
    CubeMapFBO cubeFBO;
//    unsigned drawMode = 1;
    Scene scene;
    al_sec now;
    
    int width, height;
    unsigned char* image[6];

    ShaderProgram cubemapShader;

  MyApp() {
      double world_radius = 50;

      nav().smooth(0.8);
      //lens().near(1).far(world_radius);

      // set up cube:
      cube.color(1,1,1,1);
      cube.primitive(Graphics::TRIANGLES);
      addCube(cube);
      cube.generateNormals();
  }

  virtual ~MyApp() {}

  std::string cubeFragmentCode() {

//      return AL_STRINGIFY(
//            //\n#version 330 core\n
//            //in vec3 TexCoords;
//            //out vec4 color;
//
//            uniform samplerCube skybox;
//
//            void main()
//            {
//                gl_FragColor = textureCube(skybox, gl_TexCoord[0].stp);
//            }
//      );

    return AL_STRINGIFY(uniform float lighting; uniform float texture;
                        uniform samplerCube texture0; varying vec4 color;
                        varying vec3 normal, lightDir, eyeVec; void main() {

      vec4 colorMixed;
      if (texture > 0.0) {
        vec4 textureColor = textureCube(texture0, gl_TexCoord[0].stp);
        colorMixed = mix(color, textureColor, texture);
      } else {
        colorMixed = color;
      }

      vec4 final_color = colorMixed * gl_LightSource[0].ambient;
      vec3 N = normalize(normal);
      vec3 L = lightDir;
      float lambertTerm = max(dot(N, L), 0.0);
      final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
      vec3 E = eyeVec;
      vec3 R = reflect(-L, N);
      float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
      final_color += gl_LightSource[0].specular * spec;
      gl_FragColor = mix(colorMixed, final_color, lighting);
    });
  }

  bool onCreate() {
      Shader vert, frag;
      vert.source(OmniStereo::glsl() + vertexCode(), Shader::VERTEX).compile();
      vert.printLog();
      frag.source(cubeFragmentCode(), Shader::FRAGMENT).compile();
      frag.printLog();
      cubemapShader.attach(vert).attach(frag).link();
      cubemapShader.printLog();
      cubemapShader.begin();
      cubemapShader.uniform("lighting", lightingAmount);
      cubemapShader.uniform("texture", 0.0);
      cubemapShader.end();
      
      std::cout << "OpenGL version: " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
                                                                                                     

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
          image[i] = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
          //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
      }
      
      return OmniApp::onCreate();
  }

  bool onFrame() {
      // capture the scene:
      cubeFBO.capture(gl, lens(), nav(), scene);

      // now use the captured texture:
      //gl.viewport(0, 0, width(), height());
      //gl.clearColor(0.2, 0.2, 0.2, 0);
      //gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

      //cubeFBO.drawMap(gl);

      now = MainLoop::now();

      GLint currentProgramID;
      glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramID);
      //std::cout << "onFrame program id: " << currentProgramID << std::endl;

      //return true;
      return OmniApp::onFrame();
  }

  void onDraw(Graphics& gl){

      //nav().step();

      GLint currentProgramID;
      glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgramID);
      //std::cout << "onDraw program id: " << currentProgramID << std::endl;

        glUseProgram(0);

      // first, draw it as a cylindrical map in the background:
      // ortho for 2D, ranging from 0..1 in each axis:
//      gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
//      gl.modelView(Matrix4d::identity());
//      gl.depthTesting(false);
//      gl.lighting(false);
//      gl.blending(false);
      //cubeFBO.drawMap(gl);


      // second, use it to texture a rotating object:
      //gl.projection(Matrix4d::perspective(70, width()/(double)height(), lens().near(), lens().far()));
      //gl.modelView(Matrix4d::lookAt(Vec3d(0, 0, 4), Vec3d(0, 0, 2), Vec3d(0, 1, 0)));
      
      char pixels[100 * 100 * 4];
      for (int i = 0; i < 100 * 100 * 4; i++) {
          pixels[i] = 255;
      }
      
      //if (mOmni.face() == 5) {
          glDepthMask(GL_FALSE);
          glDrawPixels(width,//mOmni.resolution(),
                       height,//mOmni.resolution(),
                       GL_RGB,
                       GL_UNSIGNED_BYTE,
                       image[mOmni.face()]);
          //std::cout << gluErrorString(glGetError()) << std::endl;
      //std::cout << mOmni.face() << std::endl;
          glDepthMask(GL_TRUE);
      //}


      cubemapShader.begin();
      cubemapShader.uniform("texture", 0.5);
      mOmni.uniforms(cubemapShader);

      gl.pushMatrix();

      // rotate over time:
      gl.rotate(now*30., 0.707, 0.707, 0.);

      gl.lighting(false);
      gl.blending(false);
      gl.depthTesting(true);

      // use cubemap texture
      cubeFBO.bind();

      // generate texture coordinates from the object:
      GLenum map = GL_OBJECT_LINEAR;
      //GLenum map = GL_REFLECTION_MAP;
      //GLenum map = GL_NORMAL_MAP;
      //GLenum map = GL_EYE_LINEAR;
      //GLenum map = GL_SPHERE_MAP;
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, map);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, map);
      glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, map);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);

      gl.draw(cube);

      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glDisable(GL_TEXTURE_GEN_R);

      cubeFBO.unbind();

      gl.popMatrix();

      //return true;

      cubemapShader.end();
  }

//  void onDraw2(Graphics& gl){
//      gl.fog(lens().far(), lens().far()/2, cubeFBO.clearColor());
//      gl.depthTesting(1);
//      //gl.draw(grid);
//      //gl.draw(mesh);
//  }

  virtual void onAnimate(al_sec dt) {
    // light.pos(nav().pos());
    pose = nav();
    // std::cout << dt << std::endl;
  }

  virtual void onSound(AudioIOData& io) {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.05;
    }
  }

  virtual void onMessage(osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const Keyboard& k) { return true; }
};

int mainOmni(int argc, char* argv[]) {
  MyApp().start();
  return 0;
}
