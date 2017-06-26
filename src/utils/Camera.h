#ifndef CAMERA_H
#define CAMERA_H

// Std. Includes
#include <vector>
#include <iostream>
#include <array>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

// Defines several possible options for camera movement. 
// Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  CW, // rotate clockwise
  CCW // rotate counter-clockwise
};

class Camera {
 public:
  // Constructor with vectors
  Camera(std::array<GLuint,2> const& screen_size,
      glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : 
    screen_size_(screen_size), position_(position),
    orientation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
    front_(glm::vec3(0.0f, 0.0f, -1.0f)), up_(up), movement_speed_(3.0f), 
    rotate_speed_(2.0f), mouse_sensitivity_(0.25f), zoom_(45.0f), 
    view_distance_(5000.0f) {
    right_ = glm::cross(front_, up_);
  }

  // Returns the current position of the camera
  inline glm::vec3 GetPosition() const {
    return position_;
  }
  
  inline void SetPosition(const glm::vec3& position) {
    position_ = position;
  }
  
  inline void SetOrientation(const glm::vec3& front, const glm::vec3& up) {
    front_ = glm::normalize(front);
    up_ = glm::normalize(up);
    right_ = glm::normalize(glm::cross(front_, up_));
  }
  
  // Returns the current angle of the camera
  inline const glm::vec3 GetEulerAngles() const {
    return glm::eulerAngles(orientation_) * 180.0f / (float) M_PI;
  }

  // Returns the view matrix
  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
  }
  
  // Returns the projection matrix
  glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(zoom_),
        (GLfloat)screen_size_[0] / 
        (GLfloat)screen_size_[1], 1.0f, view_distance_);
  }

  inline GLfloat GetZoom() const { return zoom_; }

  void SetMovementSpeed(GLfloat speed) {
    movement_speed_ = speed;
  }

  // Moves/alters the camera positions based on user input
  void Move(std::vector<bool> const& keys, float deltaTime) {
    // Move
    if(keys[GLFW_KEY_W])
      ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
      ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
      ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
      ProcessKeyboard(RIGHT, deltaTime);
    // Roll
    if(keys[GLFW_KEY_E])
      ProcessKeyboard(CW, deltaTime);
    else if(keys[GLFW_KEY_Q])
      ProcessKeyboard(CCW, deltaTime);
  }

  // Processes input received from any keyboard-like input system
  void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime) {
    GLfloat velocity = movement_speed_ * deltaTime;
    GLfloat angular_velocity = rotate_speed_ * deltaTime;
    // Move
    if (direction == FORWARD)
      position_ += front_ * velocity;
    if (direction == BACKWARD)
      position_ -= front_ * velocity;
    if (direction == LEFT)
      position_ -= right_ * velocity;
    if (direction == RIGHT)
      position_ += right_ * velocity;
    // Roll
    if (direction == CW) {
      Roll(angular_velocity);
    }
    else if (direction == CCW) {
      Roll(-angular_velocity);
    }
  }

  void Roll(GLfloat angular_velocity) {
    UpdateCameraVectors(angular_velocity, glm::normalize(front_), right_, up_);
  }

  // Processes input received from a mouse input system. Expects the offset 
  // value in both the x and y direction.
  void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset) {
    xoffset *= mouse_sensitivity_;
    yoffset *= mouse_sensitivity_;
    
    // Pitch
    UpdateCameraVectors(glm::radians(yoffset), glm::normalize(right_), up_, 
        front_);
    // Yaw
    UpdateCameraVectors(glm::radians(-xoffset), glm::normalize(up_), front_,
        right_);
  }

  // Processes input received from a mouse scroll-wheel event. Only requires 
  // input on the vertical wheel-axis
  void ProcessMouseScroll(GLfloat yoffset) {
    if (zoom_ >= 1.0f && zoom_ <= 45.0f)
      zoom_ -= yoffset;
    if (zoom_ <= 1.0f)
      zoom_ = 1.0f;
    if (zoom_ >= 45.0f)
      zoom_ = 45.0f;
  }

 private:
  // Camera Attributes
  std::array<GLuint,2> screen_size_;
  glm::vec3 position_;
  glm::quat orientation_;
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  
  // Camera options
  GLfloat movement_speed_;
  GLfloat rotate_speed_;
  GLfloat mouse_sensitivity_;
  GLfloat zoom_; // FOV
  GLfloat view_distance_;
  
  void UpdateCameraVectors(GLfloat angle, const glm::vec3& axis, 
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
};

#endif
