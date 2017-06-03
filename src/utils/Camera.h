#pragma once

// Std. Includes
#include <vector>
#include <iostream>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

// Defines several possible options for camera movement. 
// Used as abstraction to stay away from window-system specific input methods
// TODO move
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
    screen_size_(screen_size), front_(glm::vec3(0.0f, 0.0f, -1.0f)), 
    euler_(0.0f, 0.0f, 0.0f), movement_speed_(3.0f), rotate_speed_(2.0f), 
    mouse_sensitivity_(0.25f), zoom_(45.0f)
  {
    position_ = position;
    worldup_ = up;
    up_ = up;
    right_ = glm::cross(front_, up_);
    UpdateCameraVectors(0.0, {0.0, 0.0, 0.0});
  }

  // Returns the current position of the camera
  inline glm::vec3 GetPosition() const {
    return position_;
  }
  
  // Returns the current angle of the camera
  inline const glm::vec3& GetEulerAngles() const {
    return euler_;
  }

  // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
  }
  
  // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
  glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(zoom_), 
        (GLfloat)screen_size_[0] / (GLfloat)screen_size_[1], 0.1f, 500.0f);
  }

  inline GLfloat GetZoom() const { return zoom_; }

  void SetMovementSpeed(GLfloat speed) {
    movement_speed_ = speed;
  }

  // Moves/alters the camera positions based on user input
  void Move(std::vector<bool> const& keys, GLfloat deltaTime) {
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
    // Constrain the Euler angles
    euler_.z += glm::degrees(angular_velocity);
    euler_.z = std::fmod(euler_.z, 360.0f);
    if (euler_.z < 0.0f) {
      euler_.z += 360.0f;
    }
    glm::vec3 axis = front_;
    axis = glm::normalize(axis);
    UpdateCameraVectors2(angular_velocity, axis);
  }

  // Processes input received from a mouse input system. Expects the offset 
  // value in both the x and y direction.
  void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset) {
    xoffset *= mouse_sensitivity_;
    yoffset *= mouse_sensitivity_;

    GLfloat pitch_old = euler_.x;

    euler_.x += yoffset;
    euler_.y += xoffset;
    
    // Flip the world if we flip upside down
    if ((euler_.x > 90.0f && pitch_old < 90.0f) || 
        (euler_.x < 90.0f && pitch_old > 90.0f) ||
        (euler_.x > 270.0f && pitch_old < 270.0f) ||
        (euler_.x < 270.0f && pitch_old > 270.0f)) {
      // up_ = -up_;
    }
    if (euler_.x > 90.0f && euler_.x < 270.0f) {
      xoffset = -xoffset;
    }

    // Constrain the Euler angles
    euler_.x = std::fmod(euler_.x, 360.0f);
    if (euler_.x < 0.0f) {
      euler_.x += 360.0f;
    }
    euler_.y = std::fmod(euler_.y, 360.0f);
    if (euler_.y < 0.0f) {
      euler_.y += 360.0f;
    }
    
    // Pitch
    // glm::vec3 axis = glm::cross(front_, up_);
    // glm::vec3 axis = right_;
    // axis = glm::normalize(axis);
    UpdateCameraVectors(glm::radians(yoffset), glm::normalize(right_));
    // Yaw
    // UpdateCameraVectors(glm::radians(-xoffset), {0.0, 1.0, 0.0});
    UpdateCameraVectors3(glm::radians(-xoffset), glm::normalize(up_));
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
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  glm::vec3 worldup_;
  glm::vec3 euler_; // (pitch, yaw, roll)
  
  // Camera options
  GLfloat movement_speed_;
  GLfloat rotate_speed_;
  GLfloat mouse_sensitivity_;
  GLfloat zoom_; // FOV

  // Pitch
  void UpdateCameraVectors(GLfloat angle, glm::vec3 axis) {
    glm::quat rot = glm::angleAxis(angle, axis);
    glm::quat quat_view = glm::quat(0.0f, up_.x, up_.y, up_.z);
    glm::quat result = rot * quat_view * glm::conjugate(rot);

    up_.x = result.x;
    up_.y = result.y;
    up_.z = result.z;
     
    // Recalculate the right and up vector
    front_ = -glm::normalize(glm::cross(right_, up_));  
  }
 
  // Roll 
  void UpdateCameraVectors2(GLfloat angle, glm::vec3 axis) {
    glm::quat rot = glm::angleAxis(angle, axis);
    glm::quat quat_view = glm::quat(0.0f, right_.x, right_.y, right_.z);
    glm::quat result = rot * quat_view * glm::conjugate(rot);

    right_.x = result.x;
    right_.y = result.y;
    right_.z = result.z;
     
    // Recalculate the right and up vector
    up_ = glm::normalize(glm::cross(right_, front_));
  }
 
  // Yaw 
  void UpdateCameraVectors3(GLfloat angle, glm::vec3 axis) {
    glm::quat rot = glm::angleAxis(angle, axis);
    glm::quat quat_view = glm::quat(0.0f, front_.x, front_.y, front_.z);
    glm::quat result = rot * quat_view * glm::conjugate(rot);
    
    front_.x = result.x;
    front_.y = result.y;
    front_.z = result.z;
     
    // Recalculate the right and up vector
    right_ = glm::normalize(glm::cross(front_, up_));  
  }
};
