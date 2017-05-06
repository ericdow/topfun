#include "utils/CallBackWorld.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
CallBackWorld::CallBackWorld(Camera& camera) : first_mouse_(true), 
  key_state_(1024,false), last_mouse_pos_({0.0,0.0}), camera_(camera) {}

//****************************************************************************80
void CallBackWorld::ProcessKeyPress(int key, int scancode, int action, 
    int mods) {
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

  // Update the keys state machine
  if(action == GLFW_PRESS)
    key_state_[key] = true;
  else if(action == GLFW_RELEASE)
    key_state_[key] = false;	
}

//****************************************************************************80
void CallBackWorld::ProcessMouseMovement(double xpos, double ypos) {
  if(first_mouse_) {
    last_mouse_pos_ = {xpos, ypos};
    first_mouse_ = false;
  }

  GLfloat xoffset = xpos - last_mouse_pos_[0];
  GLfloat yoffset = last_mouse_pos_[1] - ypos; 
  camera_.ProcessMouseMovement(xoffset, yoffset);

  // Update the position state machine  
  last_mouse_pos_ = {xpos, ypos};
}

} // End namespace TopFun

