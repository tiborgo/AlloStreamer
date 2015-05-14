// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>

int mainCubemapsSkyboxOptimized();
int mainOmni(int argc, char* argv[]);
int mainCubemap();

int main(int argc, char* argv[])
{
    glewExperimental = GL_TRUE;

    /*GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      /*fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

        std::cout << glGetString( GL_EXTENSIONS ) << std::endl;*/

    //mainCubemapsSkyboxOptimized();
    mainOmni(argc, argv);
    //mainCubemap();
}
