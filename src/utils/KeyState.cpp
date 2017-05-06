#include "utils/KeyState.h"

namespace TopFun {

std::vector<bool> KeyState::keys = std::vector<bool>(1024, false);

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
KeyState::KeyState(GLFWwindow *window) {
  // Set the required callback functions
  glfwSetKeyCallback(window, KeyCallback);
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void KeyState::KeyCallback(GLFWwindow* window, int key, int scancode, 
    int action, int mode) {
  // Close by pressing escape key
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  // Switch between fill and line rendering
  // TODO move this...
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

  // Update the keys state machine
  if(action == GLFW_PRESS)
      keys[key] = true;
  else if(action == GLFW_RELEASE)
      keys[key] = false;	
}

} // End namespace TopFun
