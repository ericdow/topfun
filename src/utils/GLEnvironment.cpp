#include "utils/GLEnvironment.h"

namespace TopFun {
namespace GLEnvironment {

//****************************************************************************80
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, 
  int mode) {
  // Close by pressing escape key
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  // Switch between fill and line rendering
  if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
    GLint state[2];
    glGetIntegerv(GL_POLYGON_MODE, state);
    if (state[0] == GL_LINE) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
  }
}

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
  
  // Set the required callback functions
  glfwSetKeyCallback(window, KeyCallback);
  
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
