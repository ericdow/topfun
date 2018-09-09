#include "utils/GLEnvironment.h"
#include "input/CallBackWorld.h"
#include <array>

namespace TopFun {
namespace GLEnvironment {

//****************************************************************************80
GLFWwindow* SetUp(std::array<GLuint,2> const& screen_size) {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4); // controls msaa

  // Create the window
  GLFWwindow* window = glfwCreateWindow(screen_size[0], screen_size[1], 
      "TopFun", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  
  // Set the required callback functions
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, MouseCallback);

  // Initialize GLEW to setup the OpenGL Function pointers
  glewExperimental = GL_TRUE;
  glewInit();

  // Define the viewport dimensions
  glViewport(0, 0, screen_size[0], screen_size[1]);
 
  // Setup some OpenGL options
  glEnable(GL_DEPTH_TEST); // enable z-buffering
  glEnable(GL_MULTISAMPLE); // enable anti-aliasing
  glShadeModel(GL_SMOOTH);

  return window;
}

//****************************************************************************80
void SetCallback(GLFWwindow* window, CallBackWorld& callback_world) {
  // Redirect call backs to game classes
  glfwSetWindowUserPointer(window, &callback_world);
}

//****************************************************************************80
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, 
    int mods) {
  // Close by pressing escape key
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  
  // Toggle cursor lock on F6
  if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
    int cursor_mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (cursor_mode == GLFW_CURSOR_DISABLED)
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
    else
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
  }

  CallBackWorld* callback_world = 
    reinterpret_cast<CallBackWorld*>(glfwGetWindowUserPointer(window));
  callback_world->ProcessKeyPress(key, scancode, action, mods);
}

//****************************************************************************80
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
  CallBackWorld* callback_world = 
    reinterpret_cast<CallBackWorld*>(glfwGetWindowUserPointer(window));
  callback_world->ProcessMouseMovement(xpos, ypos);
}

//****************************************************************************80
glm::ivec4 GetViewport() {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  return glm::ivec4(viewport[0], viewport[1], viewport[2], viewport[3]);
}

//****************************************************************************80
void TearDown() {
  glfwTerminate();
}

} // End namespace GLEnvironment
} // End namespace TopFun
