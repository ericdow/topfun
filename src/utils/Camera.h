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
  RIGHT
};

// Default camera values
// TODO move
const GLfloat SPEED      =  3.0f;
const GLfloat SENSITIVTY =  0.25f;
const GLfloat ZOOM       =  45.0f;

const GLfloat YAW        =  -135.0f;
const GLfloat PITCH      =  0.0f;

class Camera {
 public:
  // Constructor with vectors
  Camera(GLuint sw, GLuint sh, 
      glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : 
      ScreenWidth(sw), ScreenHeight(sh), Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
      MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM) {
    this->Position = position;
    this->WorldUp = up;
    // TODO remove
    this->Yaw = YAW;
    this->Pitch = PITCH;
    this->updateCameraVectors(0.0, {0.0, 0.0, 0.0});
  }

  // Returns the current position of the camera
  inline glm::vec3 GetPosition() const {
    return Position;
  }

  // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
  }
  
  // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
  glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(Zoom), 
        (GLfloat)ScreenWidth / (GLfloat)ScreenHeight, 0.1f, 100.0f);
  }

  void SetMovementSpeed(GLfloat speed) {
    MovementSpeed = speed;
  }

  // Moves/alters the camera positions based on user input
  void Move(std::vector<bool> const& keys, GLfloat deltaTime) {
    // Camera controls
    if(keys[GLFW_KEY_W])
      ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
      ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
      ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
      ProcessKeyboard(RIGHT, deltaTime);
  }

  // Processes input received from any keyboard-like input system
  void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime) {
    GLfloat velocity = this->MovementSpeed * deltaTime;
    if (direction == FORWARD)
      this->Position += this->Front * velocity;
    if (direction == BACKWARD)
      this->Position -= this->Front * velocity;
    if (direction == LEFT)
      this->Position -= this->Right * velocity;
    if (direction == RIGHT)
      this->Position += this->Right * velocity;
  }

  // Processes input received from a mouse input system. Expects the offset 
  // value in both the x and y direction.
  void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, 
    GLboolean constrainPitch = true) {
    xoffset *= this->MouseSensitivity;
    yoffset *= this->MouseSensitivity;

    ///////////////////////////////////////////////////////////////////////
    this->Yaw   += xoffset;
    this->Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
      if (this->Pitch > 89.0f)
        this->Pitch = 89.0f;
      if (this->Pitch < -89.0f)
        this->Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Eular angles
    this->updateCameraVectors(0.0f, {0.0f, 0.0f, 0.0f});
    ///////////////////////////////////////////////////////////////////////

    /*
    glm::vec3 axis = glm::cross(Front - Position, Up);
    axis = glm::normalize(axis);
    // Update Front, Right and Up Vectors using the updated Eular angles
    updateCameraVectors(glm::radians(yoffset), axis);
    updateCameraVectors(glm::radians(-xoffset), {0.0, 1.0, 0.0});
    */
  }

  // Processes input received from a mouse scroll-wheel event. Only requires 
  // input on the vertical wheel-axis
  void ProcessMouseScroll(GLfloat yoffset) {
    if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
      this->Zoom -= yoffset;
    if (this->Zoom <= 1.0f)
      this->Zoom = 1.0f;
    if (this->Zoom >= 45.0f)
      this->Zoom = 45.0f;
  }

 private:
  // Camera Attributes
  GLuint ScreenWidth, ScreenHeight;
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  
  // Camera options
  GLfloat MovementSpeed;
  GLfloat MouseSensitivity;
  GLfloat Zoom; // FOV

  // TODO remove
  GLfloat Yaw, Pitch;

  // Calculates the front vector from the Camera's (updated) Eular Angles
  void updateCameraVectors(GLfloat angle, glm::vec3 axis) {
    /*
    glm::quat tmp, quat_view, result;
    tmp.x = axis.x * sin(angle/2);
    tmp.y = axis.y * sin(angle/2);
    tmp.z = axis.z * sin(angle/2);
    tmp.w = cos(angle/2);

    quat_view.x = Front.x;
    quat_view.y = Front.y;
    quat_view.z = Front.z;
    quat_view.w = 0.0;
  
    result = tmp * quat_view * glm::conjugate(tmp);
  
    Front.x = result.x;
    Front.y = result.y;
    Front.z = result.z;

    glm::vec3 euler_angles = eulerAngles(result);
    std::cout << euler_angles.x << std::endl;
    */
    
    ///////////////////////////////////////////////////////////////////////
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
    front.y = sin(glm::radians(this->Pitch));
    front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
    this->Front = glm::normalize(front);
    ///////////////////////////////////////////////////////////////////////
     
    // Also re-calculate the Right and Up vector
    // Normalize the vectors, because their length gets closer to 0 the more you
    // look up or down which results in slower movement.
    this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  
    this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
  }
};
