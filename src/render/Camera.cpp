#include "render/Camera.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Camera::Camera(std::array<GLuint,2> const& screen_size,
    glm::vec3 position, glm::vec3 up) : 
  screen_size_(screen_size), position_(position),
  orientation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
  front_(glm::vec3(0.0f, 0.0f, -1.0f)), up_(up), movement_speed_(3.0f), 
  rotate_speed_(2.0f), mouse_sensitivity_(0.25f), zoom_(45.0f), 
  near_(1.0f), far_(5000.0f) {
  right_ = glm::cross(front_, up_);
}

//****************************************************************************80
void Camera::Move(std::vector<bool> const& keys, float deltaTime) {
  // Move
  if(keys[GLFW_KEY_W])
    ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
  if(keys[GLFW_KEY_S])
    ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
  if(keys[GLFW_KEY_A])
    ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
  if(keys[GLFW_KEY_D])
    ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
  // Roll
  if(keys[GLFW_KEY_E])
    ProcessKeyboard(Camera_Movement::CW, deltaTime);
  else if(keys[GLFW_KEY_Q])
    ProcessKeyboard(Camera_Movement::CCW, deltaTime);
}

//****************************************************************************80
void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime) {
  GLfloat velocity = movement_speed_ * deltaTime;
  GLfloat angular_velocity = rotate_speed_ * deltaTime;
  // Move
  if (direction == Camera_Movement::FORWARD)
    position_ += front_ * velocity;
  if (direction == Camera_Movement::BACKWARD)
    position_ -= front_ * velocity;
  if (direction == Camera_Movement::LEFT)
    position_ -= right_ * velocity;
  if (direction == Camera_Movement::RIGHT)
    position_ += right_ * velocity;
  // Roll
  if (direction == Camera_Movement::CW) {
    Roll(angular_velocity);
  }
  else if (direction == Camera_Movement::CCW) {
    Roll(-angular_velocity);
  }
}

//****************************************************************************80
void Camera::Roll(GLfloat angular_velocity) {
  UpdateCameraVectors(angular_velocity, glm::normalize(front_), right_, up_);
}

//****************************************************************************80
void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset) {
  xoffset *= mouse_sensitivity_;
  yoffset *= mouse_sensitivity_;
  
  // Pitch
  UpdateCameraVectors(glm::radians(yoffset), glm::normalize(right_), up_, 
      front_);
  // Yaw
  UpdateCameraVectors(glm::radians(-xoffset), glm::normalize(up_), front_,
      right_);
}

//****************************************************************************80
void Camera::ProcessMouseScroll(GLfloat yoffset) {
  if (zoom_ >= 1.0f && zoom_ <= 45.0f)
    zoom_ -= yoffset;
  if (zoom_ <= 1.0f)
    zoom_ = 1.0f;
  if (zoom_ >= 45.0f)
    zoom_ = 45.0f;
}

//****************************************************************************80
std::array<glm::vec3,8> Camera::GetFrustumVertices() const {
  glm::mat4 inv_view_proj = 
    glm::inverse(GetProjectionMatrix() * GetViewMatrix());
  std::array<glm::vec3,8> vertices = {
    glm::vec3(-1.0f,-1.0f,-1.0f),
    glm::vec3( 1.0f,-1.0f,-1.0f),
    glm::vec3( 1.0f, 1.0f,-1.0f),
    glm::vec3(-1.0f, 1.0f,-1.0f),
    glm::vec3(-1.0f,-1.0f, 1.0f),
    glm::vec3( 1.0f,-1.0f, 1.0f),
    glm::vec3( 1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f)};
  for (auto& v : vertices) {
    glm::vec4 tmp = inv_view_proj * glm::vec4(v, 1.0f);
    v = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
  }
  return vertices;
}

//****************************************************************************80
glm::vec3 Camera::GetFrustumOrigin() const {
  glm::mat4 inv_view_proj = 
    glm::inverse(GetProjectionMatrix() * GetViewMatrix());
  glm::vec3 vert = glm::vec3(0.0f,0.0f,-1.0f);
  glm::vec4 tmp = inv_view_proj * glm::vec4(vert, 1.0f);
  vert = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
  return vert;
}

//****************************************************************************80
glm::vec3 Camera::GetFrustumTerminus() const {
  glm::mat4 inv_view_proj = 
    glm::inverse(GetProjectionMatrix() * GetViewMatrix());
  glm::vec3 vert = glm::vec3(0.0f,0.0f,1.0f);
  glm::vec4 tmp = inv_view_proj * glm::vec4(vert, 1.0f);
  vert = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
  return vert;
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Camera::UpdateCameraVectors(GLfloat angle, const glm::vec3& axis, 
    glm::vec3& dir1, glm::vec3& dir2) {
  // Rotate dir1 around axis
  glm::quat quat_rot = glm::angleAxis(angle, axis);
  glm::quat quat_dir1 = glm::quat(0.0f, dir1);
  glm::quat result = quat_rot * quat_dir1 * glm::conjugate(quat_rot);
  orientation_ = quat_rot * orientation_;

  dir1.x = result.x;
  dir1.y = result.y;
  dir1.z = result.z;
   
  // Recalculate dir2 vector
  dir2 = glm::normalize(glm::cross(dir1, axis));
}

} // End namespace TopFun
