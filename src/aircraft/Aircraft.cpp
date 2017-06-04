#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "aircraft/Aircraft.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Aircraft::Aircraft(const glm::vec3& position, const glm::quat& orientation) : 
  shader_("shaders/aircraft.vs", "shaders/aircraft.frag"),
  model_("../../../assets/models/FA-22_Raptor/FA-22_Raptor.obj"),
  position_(position), orientation_(orientation) {
}

//****************************************************************************80
void Aircraft::Draw(Camera const& camera) {
  // Activate shader
  shader_.Use();
  // Send data to the shaders
  SetShaderData(camera);
  // Draw the model
  model_.Draw(shader_);
}

//****************************************************************************80
void Aircraft::Move(std::vector<bool> const& keys, float deltaTime) {
  if(keys[GLFW_KEY_UP]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(-1.0f, 0.0f, 0.0f));
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_DOWN]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(1.0f, 0.0f, 0.0f));
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_LEFT]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(0.0f, -1.0f, 0.0f));
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_RIGHT]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(0.0f, 1.0f, 0.0f));
    Rotate(2.0f*deltaTime, axis);
  }
  glm::vec3 front = WorldToAircraft(glm::vec3(0.0f, 1.0f, 0.0f));
  position_ += 50.0f * deltaTime * front;
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Aircraft::Rotate(float angle, glm::vec3 axis) {
  axis = glm::normalize(axis);
  glm::quat quat_rot = glm::angleAxis(angle, axis);
  orientation_ = quat_rot * orientation_;
}

//****************************************************************************80
  void Aircraft::SetShaderData(Camera const& camera) {
  // Set view/projection uniforms  
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"),
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
  glm::mat4 model = glm::translate(glm::mat4(), position_);
  model *= glm::toMat4(orientation_);
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "model"),
      1, GL_FALSE, glm::value_ptr(model));

  /*
  // Set material uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), 
        "material.color"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), 
        "material.shininess"), 1.0f);

  // Set lighting uniforms
  // TODO move light direction definition to somewhere higher up
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.direction"),
      -0.3f, -1.0f, 0.0f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.ambient"), 
      0.2f, 0.2f, 0.2f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.diffuse"), 
      0.5f, 0.5f, 0.5f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.specular"), 
      1.0f, 1.0f, 1.0f);
  
  // Set the camera position uniform
  glm::vec3 camera_pos = camera.GetPosition();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "viewPos"), 
      camera_pos.x, camera_pos.y, camera_pos.z);
  */
}

} // End namespace TopFun
