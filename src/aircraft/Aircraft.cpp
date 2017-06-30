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
    position_(position), orientation_(orientation), 
    lin_momentum_(AircraftToWorld(glm::vec3(27000.0f * 150.0f, 0.0f, 0.0f), 
          orientation)), 
    ang_momentum_(0.0f, 0.0f, 0.0f),
    acceleration_(0.0f, 0.0f, 0.0f) {
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
  
  // Set the physical dimensions of the aircraft
  mass_ = 27000.0f; 
  delta_center_of_mass_ = glm::vec3(0.0f, 0.0f, -0.25f);
  inertia_[0][0] = 22000.0f;       // I_xx
  inertia_[1][1] = 162000.0f;      // I_yy
  inertia_[2][2] = 178000.0f;      // I_zz
  inertia_[0][2] = -2874.0f;       // I_xz
  inertia_[2][0] = inertia_[0][2]; // I_zx
  wetted_area_ = 316.0f;
  chord_ = 5.75f;
  span_ = 13.56f;
  dx_cg_x_ax_ = 0.05f;
  r_tail_ = glm::vec3(-4.8f, 0.0f, 0.0f);
  max_thrust_ = 311000.0f;
  
  // Define the aerodynamic coefficients
  CL_ = {0.26, 0.1, 0.2, 0.24, 0.07, 0.0, 
    -0.07, -0.14, -0.2, -0.1, -0.2, -0.3, 
    0.0, 0.35, 0.25, 0.2, 0.14, 0.07, 0.0,
    -0.07, -0.14, -0.2, -0.1, -0.2, 0.0};
  CD_ = {0.03, 0.11, 0.2, 0.4, 0.6, 0.8, 
    1.0, 0.8, 0.6, 0.4, 0.25, 0.11, // (-pi/2, 0]
    0.03, 0.11, 0.25, 0.4, 0.6, 0.8, // (0, pi/2]
    1.0, 0.8, 0.6, 0.4, 0.25, 0.11, 0.03}; // (pi/2, pi]
  CL_Q_ = 0.0f;
  Cm_Q_ = -3.6f;
  CL_alpha_dot_ = 0.72f;
  Cm_alpha_dot_ = -1.1f;
  float e = 1.0f / (1.05f + 0.007f * M_PI * span_ / chord_);
  CDi_CL2_ = 1.0f / (M_PI * e * span_ / chord_);
  CY_beta_ = -0.98f;
  Cl_beta_ = -0.12f;
  Cl_P_ = -0.26f; 
  Cl_R_ = 0.14f; 
  Cn_beta_ = 0.25f; 
  Cn_P_ = 0.022f; 
  Cn_R_ = -0.35f; 
  CL_de_ = 0.36f; 
  CD_de_ = 0.08f; 
  CY_dr_ = 0.17f; 
  Cm_de_ = -0.5f; 
  Cl_da_ = 0.08f; 
  Cn_da_ = 0.06f; 
  Cl_dr_ = -0.001f; 
  Cn_dr_ = 0.232f; 
  
  // Set the initial values for control inputs
  rudder_position_   = 0.0f;
  elevator_position_ = 0.0f;
  aileron_position_  = 0.0f;
  throttle_position_ = 1.0f;

  // Set the angle of attack so the plane is in level flight
  float dCL_dalpha0 = (CL_[1] - CL_[0]) / (2 * M_PI / (CL_.size() - 1));
  float vt = glm::l2Norm(lin_momentum_) / mass_;
  float q = 0.5f * 1.225f * vt * vt;
  float alpha0 = (mass_ * 9.81f / q / wetted_area_ - CL_[0]) / dCL_dalpha0;
  orientation_ = glm::angleAxis(alpha0,
      AircraftToWorld(glm::vec3(0.0f, 1.0f, 0.0f), orientation_)) * 
    orientation_;

  // Design Cm_ so the aircraft is stable
  glm::vec3 omega(0.0f, 0.0f, 0.0f);
  float lift0 = CalcLift(alpha0, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag0 = CalcDrag(lift0, alpha0, vt, 0.0f, q, 0.0f);
  float M_LD0 = dx_cg_x_ax_ * chord_ * (lift0*cos(alpha0) + drag0*sin(alpha0));
  float alpha1 = alpha0 + glm::radians(0.01f);
  float lift1 = CalcLift(alpha1, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag1 = CalcDrag(lift1, alpha1, vt, 0.0f, q, 0.0f);
  float M_LD1 = dx_cg_x_ax_ * chord_ * (lift1*cos(alpha1) + drag1*sin(alpha1));
  float dCm_LD_dalpha = (M_LD1-M_LD0)/(alpha1-alpha0)/q/wetted_area_/ chord_;
  float dCm_dalpha = 6.9f * dCm_LD_dalpha;
  float Cm0 = -M_LD0 / q / wetted_area_ / chord_ - dCm_dalpha * alpha0;
  Cm_ = {Cm0 - dCm_dalpha * (float)M_PI, Cm0, Cm0 + dCm_dalpha * (float)M_PI};
}

//****************************************************************************80
void Aircraft::Draw(Camera const& camera, const Sky& sky) {
  // Send data to the shaders
  SetShaderData(camera, sky);
  // Draw the model
  model_.Draw();
}

//****************************************************************************80
void Aircraft::UpdateControls(std::vector<bool> const& keys) {
  // Elevator control
  if(keys[GLFW_KEY_UP]) {
    elevator_position_ = 0.2f * elevator_position_max_;
  }
  else if(keys[GLFW_KEY_DOWN]) {
    elevator_position_ = -0.2f * elevator_position_max_;
  }
  else {
    elevator_position_ = 0.0f;
  }
  // Aileron control
  if(keys[GLFW_KEY_RIGHT]) {
    aileron_position_ = 0.5f * aileron_position_max_;
  }
  else if(keys[GLFW_KEY_LEFT]) {
    aileron_position_ = -0.5f * aileron_position_max_;
  }
  else {
    aileron_position_ = 0.0f;
  }
  // Rudder control
  if(keys[GLFW_KEY_D]) {
    rudder_position_ = 0.5f * rudder_position_max_;
  }
  else if(keys[GLFW_KEY_A]) {
    rudder_position_ = -0.5f * rudder_position_max_;
  }
  else {
    rudder_position_ = 0.0f;
  }
  // Throttle control
  if(keys[GLFW_KEY_W]) {
    throttle_position_ += 0.005;
    throttle_position_ = std::min(1.0f, throttle_position_);
  }
  else if(keys[GLFW_KEY_S]) {
    throttle_position_ -= 0.005;
    throttle_position_ = std::max(0.0f, throttle_position_);
  }
}

//****************************************************************************80
void Aircraft::operator()(const std::vector<float>& state, 
    std::vector<float>& deriv, float /* t */) {
  // Unpack the state vector
  glm::vec3 position(state[0], state[1], state[2]);
  glm::quat orientation;
  orientation.w = state[3];
  orientation.x = state[4];
  orientation.y = state[5];
  orientation.z = state[6];
  glm::vec3 lin_momentum(state[7], state[8], state[9]);
  glm::vec3 ang_momentum(state[10], state[11], state[12]);
  glm::vec3 omega = 
    glm::inverse(AircraftToWorld(inertia_, orientation)) * ang_momentum;

  // Update the forces and torques in the aircraft frame
  glm::vec3 forces, torques;
  CalcAeroForcesAndTorques(position, orientation, lin_momentum, ang_momentum,
      WorldToAircraft(omega, orientation), forces, torques);
  forces += CalcEngineForce();

  // Rotate forces and torques to world frame and add gravity
  forces = AircraftToWorld(forces, orientation);
  torques = AircraftToWorld(torques, orientation);
  forces += CalcGravityForce();

  // Update acceleration (for computing angle rates)
  acceleration_ = WorldToAircraft(forces / mass_, orientation);

  // Compute the derivative of the state vector
  for (int i = 0; i < 3; ++i) 
    deriv[i] = lin_momentum[i] / mass_;
  glm::quat omega_quat(0.0f, omega);
  glm::quat spin = 0.5f * omega_quat * orientation;
  deriv[3] = spin.w;
  deriv[4] = spin.x;
  deriv[5] = spin.y;
  deriv[6] = spin.z;
  for (int i = 0; i < 3; ++i) 
    deriv[i+7] = forces[i];
  for (int i = 0; i < 3; ++i) 
    deriv[i+10] = torques[i];

  /*
  std::cout << "velocity: " << glm::l2Norm(lin_momentum / mass_) << std::endl;
  std::cout << "torques: ";
  std::cout << torques[0] << " " << torques[1] << " " 
    << torques[2] << std::endl;
  std::cout << "ang_momentum: ";
  std::cout << ang_momentum[0] << " " << ang_momentum[1] << " " 
    << ang_momentum[2] << std::endl;
  std::cout << "spin: ";
  std::cout << spin.w << " " << spin.x << " " << spin.y << " "
    << spin.z << std::endl;
  std::cout << "orientation: ";
  std::cout << orientation.w << " " << orientation.x << " " 
    << orientation.y << " " << orientation.z << std::endl;
  std::cout << std::endl;
  */
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Aircraft::SetShaderData(const Camera& camera, const Sky& sky) {
  // Translate model to current position
  glm::mat4 model = glm::translate(glm::mat4(), position_);
  model = glm::translate(model, delta_center_of_mass_);
  // Rotate model to current orientation
  model *= glm::toMat4(orientation_);
  // Rotate model to align with aircraft axis definition
  model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
        glm::vec3(0.0f, 0.0f, 1.0f)));
  model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)));
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
void Aircraft::CalcAeroForcesAndTorques(const glm::vec3& position,
    const glm::quat& orientation, const glm::vec3& lin_momentum, 
    const glm::vec3& ang_momentum, const glm::vec3& omega, glm::vec3& forces, 
    glm::vec3& torques) const {
  glm::vec3 va = WorldToAircraft(lin_momentum / mass_, orientation);
  float vt = glm::l2Norm(va);
  if (vt > std::numeric_limits<float>::epsilon()) {
    glm::vec3 aa = WorldToAircraft(acceleration_, orientation);
    float alpha = CalcAlpha(va);
    float beta = CalcBeta(va);
    float alpha_dot = CalcAlphaDot(va, aa); 
    float dve = CalcTailVelocity(omega); 
    float rho = 1.225f * exp(-position.y / 7300.0f);
    float q = 0.5f * rho * vt * vt;

    float lift = CalcLift(alpha, alpha_dot, omega, vt, dve, q, 
        elevator_position_);
    float drag = CalcDrag(lift, alpha, vt, dve, q, elevator_position_);
    float side = CalcSideForce(beta, q, elevator_position_);
    forces.x = lift * sin(alpha) - drag * cos(alpha) - side * sin(beta);
    forces.y = side * cos(beta);
    forces.z = -lift * cos(alpha) - drag * sin(alpha);

    torques.x = CalcRollMoment(beta, omega, vt, q, aileron_position_, 
        rudder_position_);
    torques.y = CalcPitchMoment(alpha, alpha_dot, omega, vt, dve, q, 
        elevator_position_, lift, drag);
    // TODO
    static int iter = 0;
    if (iter < 4) {
      torques.y = 0.0f;
      iter++;
    }
    torques.z = CalcYawMoment(beta, omega, vt, q, aileron_position_, 
        rudder_position_);
  }
  else {
    forces = glm::vec3(0.0f, 0.0f, 0.0f);
    torques = glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

} // End namespace TopFun
