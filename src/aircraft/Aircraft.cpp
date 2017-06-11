#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "aircraft/Aircraft.h"
#include "terrain/Sky.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Aircraft::Aircraft(const glm::vec3& position, const glm::quat& orientation) :
    fuselage_shader_("shaders/aircraft.vs", "shaders/aircraft.frag"),
    canopy_shader_("shaders/aircraft.vs", "shaders/canopy.frag"),
    model_("../../../assets/models/FA-22_Raptor/FA-22_Raptor.obj"),
    position_(position), orientation_(orientation) {
  delta_center_of_mass_ = glm::vec3(0.0f, 0.0f, -0.25f);
  // Draw the canopy last since it's transparent
  std::vector<unsigned int> draw_order(22);
  std::iota(draw_order.begin(), draw_order.end(), 0);
  draw_order.back() = 2;
  draw_order[2] = draw_order.size() - 1;
  model_.SetDrawOrder(draw_order);
  // Set the shader pointers for each mesh
  std::vector<Shader*> shaders(22);
  for (size_t i = 0; i < shaders.size()-1; i++) {
    shaders[i] = &fuselage_shader_;
  }
  shaders.back() = &canopy_shader_;
  model_.SetShaders(shaders);
  // Define the aerodynamic coefficients
  // TODO
  CL_ = {0.0, 1.0, 0.0};
}

//****************************************************************************80
void Aircraft::Draw(Camera const& camera, const Sky& sky) {
  // Send data to the shaders
  SetShaderData(camera, sky);
  // Draw the model
  model_.Draw();
}

//****************************************************************************80
void Aircraft::Move(std::vector<bool> const& keys, float deltaTime) {
  if(keys[GLFW_KEY_UP]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(-1.0f, 0.0f, 0.0f), 
        orientation_);
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_DOWN]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(1.0f, 0.0f, 0.0f), orientation_);
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_LEFT]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(0.0f, -1.0f, 0.0f),
        orientation_);
    Rotate(2.0f*deltaTime, axis);
  }
  if(keys[GLFW_KEY_RIGHT]) {
    glm::vec3 axis = WorldToAircraft(glm::vec3(0.0f, 1.0f, 0.0f), orientation_);
    Rotate(2.0f*deltaTime, axis);
  }
  glm::vec3 front = WorldToAircraft(glm::vec3(0.0f, 1.0f, 0.0f), orientation_);
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
void Aircraft::SetShaderData(const Camera& camera, const Sky& sky) {
  // Rotate and translate model 
  glm::mat4 model = glm::translate(glm::mat4(), position_);
  model = glm::translate(model, delta_center_of_mass_);
  model *= glm::toMat4(orientation_);
  model = glm::translate(model, -delta_center_of_mass_);
  
  // Set model/view/projection uniforms
  fuselage_shader_.Use();
  glUniformMatrix4fv(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(fuselage_shader_.GetProgram(),
        "projection"), 1, GL_FALSE, 
      glm::value_ptr(camera.GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "model"), 1, GL_FALSE, glm::value_ptr(model));
  canopy_shader_.Use();
  glUniformMatrix4fv(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(canopy_shader_.GetProgram(),
        "projection"), 1, GL_FALSE, 
      glm::value_ptr(camera.GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "model"), 1, GL_FALSE, glm::value_ptr(model));

  /*
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

//****************************************************************************80
void Aircraft::CalcAeroForcesAndMoments(const glm::vec3& lin_momentum, 
    const glm::vec3& ang_momentum, const glm::vec3& acceleration,
    const glm::quat orientation, float da, float de, float dr) const {
  glm::vec3 Forces;
  glm::vec3 Moments;
  glm::vec3 va = WorldToAircraft(lin_momentum / mass_, orientation);
  glm::vec3 omega = WorldToAircraft(inverse_inertia_*ang_momentum, orientation);
  float alpha = CalcAlpha(va);
  float beta = CalcBeta(va);
  float alpha_dot = CalcAlphaDot(va, acceleration); 
  float dve = CalcTailVelocity(omega); 
  float vt = glm::l2Norm(va);
  float rho = 1.225; // TODO
  float q = 0.5*rho*vt*vt;
  float Lift = CalcLift(alpha, alpha_dot, omega, vt, dve, q, de);
  float Drag = CalcDrag(alpha, vt, dve, q, de);
  float Side = CalcSideForce(beta, q, de);
  Forces.x = Lift * sin(alpha) - Drag * cos(alpha) - Side * sin(beta);
  Forces.y = Side * cos(beta);
  Forces.z = -Lift * cos(alpha) - Drag * sin(alpha);
}

} // End namespace TopFun
