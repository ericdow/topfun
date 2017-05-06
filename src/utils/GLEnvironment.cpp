#ifndef GLENVIRONMENT_H
#define GLENVIRONMENT_H

#include "utils/GLEnvironment.h"

namespace TopFun {
namespace GLEnvironment {

//****************************************************************************80
GLFWwindow* SetUp(GLuint screenWidth, GLuint screenHeight) {
  // Init GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "TopFun", 
      nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // Options
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Initialize GLEW to setup the OpenGL Function pointers
  glewExperimental = GL_TRUE;
  glewInit();

  // Define the viewport dimensions
  glViewport(0, 0, screenWidth, screenHeight);
  
  // Setup some OpenGL options
  glEnable(GL_DEPTH_TEST);

  return window;
}

//****************************************************************************80
void TearDown() {
  glfwTerminate();
}

} // End namespace GLEnvironment
} // End namespace TopFun

#endif
