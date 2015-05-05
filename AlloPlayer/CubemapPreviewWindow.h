#pragma once

// Std. Includes
#include <string>
#include <algorithm>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "shader.h"
#include "camera.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

#include <boost/thread.hpp>

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

class CubemapPreviewWindow
{

public:

	CubemapPreviewWindow();

	// Function prototypes
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void Do_Movement();
	GLuint loadTexture(GLchar* path);
	GLuint loadCubemap(std::vector<const GLchar*> faces);

	void gameLoop();

	GLFWwindow* window;
	Shader* shader;
	Shader* skyboxShader;
	GLuint cubeVAO, cubeVBO;
	GLuint skyboxTexture;
	GLuint skyboxVAO, skyboxVBO;

	boost::thread gameThread;

};