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

namespace TopFun {

class Camera {
 private:
  // Defines several possible options for camera movement. 
  // Used as abstraction to stay away from window-system specific input methods
  enum class Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    CW, // rotate clockwise
    CCW // rotate counter-clockwise
  };
  
 public:
  // Constructor with vectors
  Camera(std::array<GLuint,2> const& screen_size,
      glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)); 

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
  
  inline std::array<GLfloat,6> GetOrientation() {
    std::array<GLfloat,6> o;
    for (int d = 0; d < 3; ++d) {
      o[d] = front_[d];
      o[d+3] = up_[d];
    }
    return o;
  }
  
  // Returns the current angle of the camera
  inline const glm::vec3 GetEulerAngles() const {
    return glm::eulerAngles(orientation_) * 180.0f / (float) M_PI;
  }

  // Returns the view matrix
  inline glm::mat4 GetViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
  }
  
  // Returns the projection matrix
  inline glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(zoom_),
        (GLfloat)screen_size_[0] / 
        (GLfloat)screen_size_[1], near_, far_);
  }

  inline GLfloat GetZoom() const { return zoom_; }

  inline const glm::vec3& GetFront() const { return front_; }

  inline const std::array<float,2> GetNearFar() const { return {near_, far_}; }

  glm::vec3 GetFrustumOrigin() const;
  
  glm::vec3 GetFrustumTerminus() const;

  inline void SetMovementSpeed(GLfloat speed) {
    movement_speed_ = speed;
  }

  // Moves/alters the camera positions based on user input
  void Move(std::vector<bool> const& keys, float deltaTime);

  // Processes input received from any keyboard-like input system
  void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);

  void Roll(GLfloat angular_velocity);

  // Processes input received from a mouse input system. Expects the offset 
  // value in both the x and y direction.
  void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);

  // Processes input received from a mouse scroll-wheel event. Only requires 
  // input on the vertical wheel-axis
  void ProcessMouseScroll(GLfloat yoffset);
  
  std::array<glm::vec3,8> GetFrustumVertices() const;

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
  GLfloat near_; // near plane distance
  GLfloat far_; // far plane distance
  
  void UpdateCameraVectors(GLfloat angle, const glm::vec3& axis, 
      glm::vec3& dir1, glm::vec3& dir2);
};

} // End namespace TopFun

#endif
