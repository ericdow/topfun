#include "input/CallBackWorld.h"
#include "render/ShadowCascadeRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
CallBackWorld::CallBackWorld(Camera& camera, DebugOverlay& debug_overlay,
    ShadowCascadeRenderer& shadow_renderer,
    std::array<GLuint,2> const& screen_size) : 
  first_mouse_(true), last_mouse_pos_({(double)screen_size[0]/2, 
  (double)screen_size[1]/2}), key_state_(1024,false), fps_locked_(true), 
  w_double_pressed_(false), last_w_press_time_(-100.0f),
  camera_(camera), debug_overlay_(debug_overlay), 
  shadow_renderer_(shadow_renderer) {}

//****************************************************************************80
void CallBackWorld::ProcessKeyPress(int key, int /* scancode */, int action, 
    int /* mods */) {
  // Switch between fill and line rendering on F1
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
  
  // Toggle debug overlay on F2
  if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
    debug_overlay_.ToggleVisible();
  }
  
  // Toggle FPS lock on F3
  if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
    fps_locked_ = !fps_locked_;
  }
  
  // Toggle depth map overlay on F4
  if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
    shadow_renderer_.ToggleVisible();
  }
  
  // Check for double press on W
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    float w_press_time = glfwGetTime();
    if (w_press_time - last_w_press_time_ < 0.2f) {
      w_double_pressed_ = true;
    }
    last_w_press_time_ = w_press_time;
  }
  if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
    w_double_pressed_ = false;
  }

  // Set camera movement speed
  if (w_double_pressed_) {
    camera_.SetMovementSpeed(18.0);
  }
  else {
    camera_.SetMovementSpeed(6.0);
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

  // Update the position state machine  
  last_mouse_pos_ = {xpos, ypos};
  
  camera_.ProcessMouseMovement(xoffset, yoffset);
}

} // End namespace TopFun

