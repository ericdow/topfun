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
  // Set the physical dimensions of the aircraft
  mass_ = 40000.0f; 
  delta_center_of_mass_ = glm::vec3(0.0f, 0.0f, -0.25f);
  inertia_[0][0] = 22000.0f;       // I_xx
  inertia_[1][1] = 162000.0f;      // I_yy
  inertia_[2][2] = 178000.0f;      // I_zz
  inertia_[0][2] = -2874.0f;       // I_xz
  inertia_[2][0] = inertia_[0][2]; // I_zx
  inverse_inertia_ = glm::inverse(inertia_);
  wetted_area_ = 316.0f;
  chord_ = 5.75f;
  span_ = 13.56f;
  r_tail_ = glm::vec3(-4.8f, 0.0f, 0.0f);
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
    const glm::quat orientation, float da, float de, float dr,
    glm::vec3& Forces, glm::vec3& Moments) const {
  glm::vec3 va = WorldToAircraft(lin_momentum / mass_, orientation);
  glm::vec3 omega = WorldToAircraft(inverse_inertia_*ang_momentum, orientation);
  float alpha = CalcAlpha(va);
  float beta = CalcBeta(va);
  float alpha_dot = CalcAlphaDot(va, acceleration); 
  float dve = CalcTailVelocity(omega); 
  float vt = glm::l2Norm(va);
  float rho = 1.225; // TODO
  float q = 0.5*rho*vt*vt;

  float lift = CalcLift(alpha, alpha_dot, omega, vt, dve, q, de);
  float drag = CalcDrag(alpha, vt, dve, q, de);
  float side = CalcSideForce(beta, q, de);
  Forces.x = lift * sin(alpha) - drag * cos(alpha) - side * sin(beta);
  Forces.y = side * cos(beta);
  Forces.z = -lift * cos(alpha) - drag * sin(alpha);

  Moments.x = CalcRollMoment(beta, omega, vt, q, da, dr);
  Moments.y = CalcPitchMoment(alpha, alpha_dot, omega, vt, dve, q, de);
  Moments.z = CalcYawMoment(beta, omega, vt, q, da, dr);
}

} // End namespace TopFun
