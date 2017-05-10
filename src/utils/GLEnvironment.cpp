#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"

namespace TopFun {
namespace GLEnvironment {

//****************************************************************************80
GLFWwindow* SetUp(GLuint screenWidth, GLuint screenHeight, 
    CallBackWorld& call_back_world) {
  // Initialize GLFW
  glfwInit(); // TODO this leaks apparently...
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create the window
  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "TopFun", 
      nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // Redirect call backs to game classes
  glfwSetWindowUserPointer(window, &call_back_world);
  
  // Set the required callback functions
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, MouseCallback);

  // Make sure cursor is hidden and stays in window
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

  // Initialize GLEW to setup the OpenGL Function pointers
  glewExperimental = GL_TRUE;
  glewInit();

  // Define the viewport dimensions
  glViewport(0, 0, screenWidth, screenHeight);
  
  // Setup some OpenGL options
  glEnable(GL_DEPTH_TEST); // enable z-buffering

  return window;
}

//****************************************************************************80
void TearDown() {
  glfwTerminate();
}

//****************************************************************************80
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, 
    int mods) {
  // Close by pressing escape key
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  CallBackWorld* call_back_world = 
    reinterpret_cast<CallBackWorld*>(glfwGetWindowUserPointer(window));
  call_back_world->ProcessKeyPress(key, scancode, action, mods);
}

//****************************************************************************80
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
  CallBackWorld* call_back_world = 
    reinterpret_cast<CallBackWorld*>(glfwGetWindowUserPointer(window));
  call_back_world->ProcessMouseMovement(xpos, ypos);
}

} // End namespace GLEnvironment
} // End namespace TopFun
