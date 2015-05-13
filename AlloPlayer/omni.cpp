#include "alloutil/al_OmniApp.hpp"
#include "alloutil/al_TextureGL.hpp"
#include "alloutil/al_CubeMapFBO.hpp"
using namespace al;

// GLFW
#include <GLFW/glfw3.h>

#include <SOIL.h>

extern GLuint screenWidth, screenHeight;

GLuint skyboxVAO, skyboxVBO;

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

// Loads a cubemap texture from 6 individual texture faces
// Order should be:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)? (CHECK THIS)
// -Z (back)?
GLuint loadCubemap(std::vector<const GLchar*> faces);
/*{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

    int width, height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for (GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}*/

class H264StreamCubeMapTexture: public CubeMapTexture {

public:
    H264StreamCubeMapTexture(int resolution) : CubeMapTexture(resolution) {

    }

    virtual void onCreate() {
        CubeMapTexture::onCreate();
        bind();

        std::vector<const GLchar*> faces;
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/right.jpg");
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/left.jpg");
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/top.jpg");
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/bottom.jpg");
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/back.jpg");
        faces.push_back(STR(AlloPlayer_RESOURCE_DIR) "skybox/front.jpg");

        int width, height;
        unsigned char* image;

        for (GLuint i = 0; i < faces.size(); i++)
        {
            image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        }

        std::cout << width << ", " << height << std::endl;
        /*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);*/
    }
};

struct MyApp : OmniApp, public Drawable {
  Mesh sphere, cube;
  Light light;
  GLuint skyboxTexture;
  H264StreamCubeMapTexture cubemapTexture;
  CubeMapFBO cubeFBO;
  Graphics gl;

   MyApp() : cubemapTexture(2048) {
    sphere.primitive(Graphics::TRIANGLES);
    addSphere(sphere, 1.0, 32, 32);
    for (int i = 0; i < sphere.vertices().size(); ++i) {
      float f = (float)i / sphere.vertices().size();
      sphere.color(Color(HSV(f, 1 - f, 1), 1));
    }
    sphere.generateNormals();
    light.ambient(Color(0.4, 0.4, 0.4, 1.0));
    light.pos(5, 5, 5);

    //initAudio();


    // Initialize GLEW to setup the OpenGL Function pointers

    //glewInit();

    glfwInit();
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

    //GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
    //glfwMakeContextCurrent(window);

    char* extensions = (char*)glGetString( GL_EXTENSIONS );

    std::cout << "extensions: " << (bool)glewExperimental << std::endl;

    glewInit();

    GLfloat skyboxVertices[] = {
        // Positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    // Setup skybox VAO

    //glGenVertexArrays(1, &skyboxVAO);
    /*glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);*/

    cube.color(1,1,1,1);
    cube.primitive(Graphics::TRIANGLES);
    addCube(cube, false, 5.0);
    cube.generateNormals();

    //skyboxTexture = loadCubemap(faces);
  }

  virtual ~MyApp() {}

  bool onFrame2(){
      OmniApp::onFrame();
      //nav.step();


      // capture the scene:
      cubeFBO.capture(gl, lens(), nav(), *this);

      // now use the captured texture:
      gl.viewport(0, 0, width(), height());
      gl.clearColor(0.2, 0.2, 0.2, 0);
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

      // first, draw it as a cylindrical map in the background:
      // ortho for 2D, ranging from 0..1 in each axis:
      gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
      gl.modelView(Matrix4d::identity());
      gl.depthTesting(false);
      gl.lighting(false);
      gl.blending(false);
      cubeFBO.drawMap(gl);


      // second, use it to texture a rotating object:
      //gl.projection(Matrix4d::perspective(70, width()/(double)height(), lens().near(), lens().far()));
      //gl.modelView(Matrix4d::lookAt(Vec3d(0, 0, 4), Vec3d(0, 0, 2), Vec3d(0, 1, 0)));

      // rotate over time:
      //gl.rotate(MainLoop::now()*30., 0.707, 0.707, 0.);

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

      return true;
  }

  virtual void onDraw(Graphics& g) {
    light();
    // say how much lighting you want
    shader().uniform("lighting", 1.0);

    g.draw(sphere);

    cubemapTexture.drawMap(g);

    //cubemapTexture.bind();

    // generate texture coordinates from the object:
    //GLenum map = GL_OBJECT_LINEAR;
    //GLenum map = GL_REFLECTION_MAP;
    //GLenum map = GL_NORMAL_MAP;
    //GLenum map = GL_EYE_LINEAR;
    //GLenum map = GL_SPHERE_MAP;
    /*glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, map);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, map);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, map);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    g.draw(cube);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    cubemapTexture.unbind();*/

    //glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    g.draw(cube);
  }

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
