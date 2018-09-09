#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <limits>

#include "aircraft/Aircraft.h"
#include "sky/Sky.h"
#include "render/ShadowCascadeRenderer.h"
#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Aircraft::Aircraft(const glm::dvec3& position, const glm::quat& orientation,
    const Camera& camera, const Terrain& terrain) :
    camera_(camera), terrain_(terrain),
    fuselage_shader_("shaders/aircraft.vs", "shaders/aircraft.fs"),
    canopy_shader_("shaders/aircraft.vs", "shaders/canopy.fs"),
    exhaust_shader_("shaders/exhaust.vs", "shaders/exhaust.fs"),
     model_("../../../assets/models/FA-22_Raptor/FA-22_Raptor.obj"),
    //model_("../../../assets/models/Box/FA-22_Raptor.obj"),
    collision_model_(
         "../../../assets/models/FA-22_Raptor/FA-22_Raptor_Convex_Hull.obj",
         true),
        //"../../../assets/models/Box/FA-22_Raptor_Convex_Hull.obj", true),
    position_(position), orientation_(orientation), 
    lin_momentum_(AircraftToWorld(glm::vec3(27000.0f * 150.0f, 0.0f, 0.0f), 
          orientation)), 
    ang_momentum_(0.0f, 0.0f, 0.0f),
    acceleration_(0.0f, 0.0f, 0.0f), 
    crashed_(false) {
  // Draw the canopy last since it's transparent
  std::vector<unsigned int> draw_order(model_.GetNumMeshes());
  std::iota(draw_order.begin(), draw_order.end(), 0);
  draw_order.back() = 2;
  draw_order[2] = draw_order.size() - 1;
  model_.SetDrawOrder(draw_order);
  
  // Set the shader pointers for each mesh
  std::vector<const Shader*> shaders(model_.GetNumMeshes(), &fuselage_shader_);
  shaders[2] = &canopy_shader_;
  model_.SetShaders(shaders);

  // Set the mesh indices for the various aircraft components
  rudder_mesh_indices_ = {17, 16}; // left, right
  aileron_mesh_indices_ = {13, 12}; // left, right
  elevator_mesh_indices_ = {8, 9}; // left, right
  airframe_mesh_indices_ = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 
    14, 15, 18, 19, 20, 21};
  
  // Set the physical dimensions of the aircraft
  mass_ = 27000.0f;
  inv_mass_ = 1.0f / mass_;
  // y+/-: for/aft, z+/-: up-down
  delta_center_of_mass_ = glm::dvec3(0.0f, 0.0f, -1.0f);
  inertia_[0][0] = 22000.0f;       // I_xx
  inertia_[1][1] = 162000.0f;      // I_yy
  inertia_[2][2] = 178000.0f;      // I_zz
  inertia_[0][2] = -2874.0f;       // I_xz
  inertia_[2][0] = inertia_[0][2]; // I_zx
  e_collision_ = 0.2f;
  mu_static_ = 1.0f;
  mu_dynamic_ = 0.3f;
  wetted_area_ = 316.0f;
  chord_ = 5.75f;
  span_ = 13.56f;
  dx_cg_x_ax_ = 0.05f;
  r_tail_ = glm::vec3(-4.8f, 0.0f, 0.0f);
  max_thrust_ = 311000.0f;
  rudder_axis_ = {{glm::vec3(-1.57663f, -6.48513f, -0.12633f),
    glm::vec3(-0.410372f, 0.414351f, 0.812345f)}};
  aileron_axis_ = {{glm::vec3(-4.53069f, -4.33568f, -0.668355f),
    glm::vec3(-0.958864f, 0.277462f, -0.0599722f)}};
  elevator_axis_ = {{glm::vec3(-2.00298f, -6.8562f, -0.625643f),
    glm::vec3(-0.994531f, -0.104437f, 0.0f)}};

  // Define the aerodynamic performance coefficients
  CL_ = {0.26, 0.1, 0.2, 0.24, 0.07, 0.0, // (-pi/2, 0] 
    -0.03, -0.14, -0.2, -0.1, -0.2, 0.0, // (0, pi/2]
    0.0, 0.55, 0.45, 0.3, 0.14, 0.07, 0.0, // (pi/2, pi]
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
  CL_de_ = 0.12f; 
  CD_de_ = 0.08f; 
  CY_dr_ = 0.12f; 
  Cm_de_ = -0.4f; 
  Cl_da_ = 0.02f; 
  Cn_da_ = 0.06f; 
  Cl_dr_ = -0.001f; 
  Cn_dr_ = 0.04f; 
  
  // Set the initial values for control inputs
  rudder_position_   = 0.0f;
  elevator_position_ = 0.0f;
  aileron_position_  = 0.0f;
  throttle_position_ = 1.0f;

  // Design Cm_ so the aircraft is stable
  float dCL_dalpha0 = (CL_[1] - CL_[0]) / (2 * M_PI / (CL_.size() - 1));
  float vt = glm::l2Norm(lin_momentum_) * inv_mass_;
  float q = 0.5f * 1.225f * vt * vt;
  float alpha0 = (mass_ * 9.81f / q / wetted_area_ - CL_[0]) / dCL_dalpha0;
  glm::vec3 omega(0.0f, 0.0f, 0.0f);
  float lift0 = CalcLift(alpha0, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag0 = CalcDrag(lift0, alpha0, vt, 0.0f, q, 0.0f);
  float M_LD0 = dx_cg_x_ax_ * chord_ * (lift0*cos(alpha0) + drag0*sin(alpha0));
  float alpha1 = alpha0 + glm::radians(1.0f);
  float lift1 = CalcLift(alpha1, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag1 = CalcDrag(lift1, alpha1, vt, 0.0f, q, 0.0f);
  float M_LD1 = dx_cg_x_ax_ * chord_ * (lift1*cos(alpha1) + drag1*sin(alpha1));
  float dCm_LD_dalpha = (M_LD1-M_LD0)/(alpha1-alpha0)/q/wetted_area_/ chord_;
  float dCm_dalpha = 8.0f * dCm_LD_dalpha; // increasing this causes nose up
  float Cm0 = -M_LD0 / q / wetted_area_ / chord_ - dCm_dalpha * alpha0;
  int npts = 180/15;
  Cm_.resize(2*npts + 1);
  Cm_[npts] = Cm0;
  for (int i = 0; i < npts; ++i) {
    float slope_factor = std::pow(1.0f - 0.9 * i / npts, 3.0);
    Cm_[npts-i-1] = Cm_[npts-i] - slope_factor * dCm_dalpha * (float)M_PI / npts;
    Cm_[npts+i+1] = Cm_[npts+i] + slope_factor * dCm_dalpha * (float)M_PI / npts;
  }

  // Set up the data for drawing the exhaust
  delta_exhaust_ = {0.637885f, -6.717596f, -0.562625f};
  delta_flame_ = {0.637885f, -6.217596f, -0.593625f};
  r_flame_ = 0.55f;
  SetupDrawData();

  // Set up the audio data
  engine_idle_.SetBuffer("engine_idle");
  engine_idle_.SetLooping(true);
  engine_idle_.SetGain(0.0f);
  engine_idle_.SetRollOff(0.2f);
  engine_idle_.SetReferenceDistance(20.0f);
  engine_idle_.Play();
  
  afterburner_.SetBuffer("afterburner");
  afterburner_.SetLooping(true);
  afterburner_.SetGain(0.0f);
  afterburner_.SetRollOff(0.2f);
  afterburner_.SetReferenceDistance(20.0f);
  afterburner_.Play();
  
  // Determine which joystick to use 
  // TODO move this...
  joystick_id_ = -1;
  const char* joystick_name = "Saitek";
  for (int i = 0; i < GLFW_JOYSTICK_LAST; ++i) {
    if (glfwGetJoystickName(i)) {
      if (std::strstr(glfwGetJoystickName(i), joystick_name)) {
        joystick_id_ = i;
        std::cout << "Using controller " << glfwGetJoystickName(i) << std::endl;
      }
    }
  }
  // TODO
  // TODO
  // TODO
  // TODO
  lin_momentum_ *= 0.0;
  position_.y = 10.0;
  throttle_position_ = 0.0;
}

//****************************************************************************80
Aircraft::~Aircraft() {
  glDeleteVertexArrays(1, &sphere_VAO_);
}

//****************************************************************************80
void Aircraft::Draw(const Sky& sky, 
    const ShadowCascadeRenderer* pshadow_renderer, const Shader* shader) {
  if (!shader) {
    // Send data to the shaders
    SetShaderData(sky, *pshadow_renderer);
  }

  // Send the model orientation info
  glm::mat4 cm_model = GetAircraftModelMatrix();
  for (int i : airframe_mesh_indices_) {
    model_.SetModelMatrix(&cm_model, i);
  }
  glm::mat4 left_rudder_model = GetControlSurfaceModelMatrix( 
      rudder_axis_[0], rudder_axis_[1], 
      rudder_position_ * rudder_position_max_);
  model_.SetModelMatrix(&left_rudder_model, rudder_mesh_indices_[0]);
  glm::mat4 right_rudder_model = GetControlSurfaceModelMatrix( 
      rudder_axis_[0], 
      rudder_axis_[1], rudder_position_ * rudder_position_max_, true);
  model_.SetModelMatrix(&right_rudder_model, rudder_mesh_indices_[1]);
  glm::mat4 left_aileron_model = GetControlSurfaceModelMatrix(
      aileron_axis_[0], aileron_axis_[1], 
      -aileron_position_ * aileron_position_max_);
  model_.SetModelMatrix(&left_aileron_model, aileron_mesh_indices_[0]);
  glm::mat4 right_aileron_model = GetControlSurfaceModelMatrix(
      aileron_axis_[0], aileron_axis_[1], 
      -aileron_position_ * aileron_position_max_, true);
  model_.SetModelMatrix(&right_aileron_model, aileron_mesh_indices_[1]);
  glm::mat4 left_elevator_model = GetControlSurfaceModelMatrix(
      elevator_axis_[0], elevator_axis_[1], 
      -elevator_position_ * elevator_position_max_);
  model_.SetModelMatrix(&left_elevator_model, elevator_mesh_indices_[0]);
  glm::mat4 right_elevator_model = GetControlSurfaceModelMatrix(
      elevator_axis_[0], elevator_axis_[1], 
      elevator_position_ * elevator_position_max_, true);
  model_.SetModelMatrix(&right_elevator_model, elevator_mesh_indices_[1]);

  // Enable face-culling and blending (for cockpit drawing) 
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Draw the model
  model_.Draw(shader);

  // Disable face-culling
  glDisable(GL_CULL_FACE);

  // Draw the exhaust last
  if (!shader) {
    DrawExhaust();
  }
  
  // Disable blending
  glDisable(GL_BLEND);
}

//****************************************************************************80
void Aircraft::UpdateControls(std::vector<bool> const& keys) {
  // Check for joystick to determine input mode
  if (glfwJoystickPresent(joystick_id_) == GL_TRUE) {
    // Grab joystick state and set control surfaces/throttle
    int num_axes;
    const float* axes = glfwGetJoystickAxes(joystick_id_, &num_axes);
    aileron_position_  = axes[0] * aileron_position_max_;
    elevator_position_ = -axes[1] * elevator_position_max_;
    throttle_position_ = 0.5f * (1.0f - axes[2]);
    rudder_position_   = axes[5] * rudder_position_max_;
  }
  else {
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
    if(keys[GLFW_KEY_EQUAL]) {
      throttle_position_ += 0.005;
      throttle_position_ = std::min(1.0f, throttle_position_);
    }
    else if(keys[GLFW_KEY_MINUS]) {
      throttle_position_ -= 0.005;
      throttle_position_ = std::max(0.0f, throttle_position_);
    }
  }
  // Update audio levels, etc.
  UpdateEngineSounds();
}

//****************************************************************************80
std::vector<double> Aircraft::GetStateDerivative(
    const std::vector<double>& state, float /* t */) {
  // Unpack the state vector
  glm::vec3 position(state[0], state[1], state[2]);
  glm::quat orientation = glm::quat(state[3], state[4], state[5], state[6]);
  glm::vec3 lin_momentum(state[7], state[8], state[9]);
  glm::vec3 ang_momentum(state[10], state[11], state[12]);
  glm::vec3 omega = GetAngularVelocity(orientation, ang_momentum); 

  // Update the forces and torques in the aircraft frame
  CalcAeroForcesAndTorques(position, orientation, lin_momentum, 
      WorldToAircraft(omega, orientation), forces_, torques_);
  forces_ += CalcEngineForce();

  // Rotate forces and torques to world frame and add gravity
  forces_ = AircraftToWorld(forces_, orientation);
  torques_ = AircraftToWorld(torques_, orientation);
  forces_ += CalcGravityForce();

  // Update acceleration (for computing angle rates)
  acceleration_ = WorldToAircraft(forces_ * inv_mass_, orientation);

  // Compute the derivative of the state vector
  std::vector<double> deriv(13);
  for (int i = 0; i < 3; ++i) 
    deriv[i] = lin_momentum[i] * inv_mass_;
  glm::quat omega_quat(0.0f, omega);
  glm::quat spin = 0.5f * omega_quat * orientation;
  deriv[3] = spin.w;
  deriv[4] = spin.x;
  deriv[5] = spin.y;
  deriv[6] = spin.z;
  for (int i = 0; i < 3; ++i) 
    deriv[i+7] = forces_[i];
  for (int i = 0; i < 3; ++i) 
    deriv[i+10] = torques_[i];
  return deriv;
}
  
//****************************************************************************80
void Aircraft::DoPhysicsStep(float t, float dt) {
  // Compute the initial state derivative
  auto state = GetState();
  auto const deriv = GetStateDerivative(state, t);
  
  // Update momentum due to forces/torques
  lin_momentum_ += forces_ * dt;
  ang_momentum_ += torques_ * dt;

  // Get the current set of contacts
  for (int i = 0; i < 3; ++i)
    state[i] += lin_momentum_[i] * inv_mass_ * dt;
  {
  glm::vec3 omega = GetAngularVelocity(orientation_, ang_momentum_);
  glm::quat omega_quat(0.0f, omega);
  glm::quat spin = 0.5f * omega_quat * orientation_;
  state[3] += spin.w * dt;
  state[4] += spin.x * dt;
  state[5] += spin.y * dt;
  state[6] += spin.z * dt;
  }
  auto contacts = GetContacts(state, dt);

  // Iterate to solve for the new velocities
  const int max_iter = 20;
  float j_n(0.0f), j_t(0.0f); // no warm-starting
  for (int i = 0; i < max_iter; ++i) {
    for (std::size_t c = 0; c < contacts.size(); ++c) {
      // Apply normal impulse
      auto v = GetVelocity() + glm::cross(GetAngularVelocity(orientation_,
            ang_momentum_), contacts[c].r);
      auto vn = glm::dot(v, contacts[c].n);
      float dj_n = contacts[c].mass_n * (-vn + contacts[c].bias);
      float j_n0 = j_n;
      j_n = std::max(j_n0 + dj_n, 0.0f);
      dj_n = j_n - j_n0;
      lin_momentum_ += dj_n * contacts[c].n;
      ang_momentum_ += dj_n * glm::cross(contacts[c].r, contacts[c].n);

      // Apply tangent impulse
      v = GetVelocity() + glm::cross(GetAngularVelocity(orientation_,
            ang_momentum_), contacts[c].r);
      auto vt = glm::dot(v, contacts[c].t);
      float dj_t = contacts[c].mass_t * -vt;
      float j_t_max = mu_dynamic_ * j_n;
      float j_t0 = j_t;
      j_t = std::min(std::max(j_t0 + dj_t, -j_t_max), j_t_max);
      dj_t = j_t - j_t0;
      lin_momentum_ += dj_t * contacts[c].t;
      ang_momentum_ += dj_t * glm::cross(contacts[c].r, contacts[c].t);
    }
  }

  // std::cout << j_n/dt << " " << mass_ * 9.81 << std::endl;
  
  // Update positions
  position_ += lin_momentum_ * inv_mass_ * dt;
  glm::vec3 omega = GetAngularVelocity(orientation_, ang_momentum_);
  glm::quat omega_quat(0.0f, omega);
  glm::quat spin = 0.5f * omega_quat * orientation_;
  orientation_ += spin * dt;
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Aircraft::CalcAeroForcesAndTorques(const glm::vec3& position,
    const glm::quat& orientation, const glm::vec3& lin_momentum, 
    const glm::vec3& omega, glm::vec3& forces, 
    glm::vec3& torques) const {
  glm::vec3 va = WorldToAircraft(lin_momentum * inv_mass_, orientation);
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
    torques.z = CalcYawMoment(beta, omega, vt, q, aileron_position_, 
        rudder_position_);
  }
  else {
    forces = glm::vec3(0.0f, 0.0f, 0.0f);
    torques = glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

//****************************************************************************80
void Aircraft::SetShaderData(const Sky& sky,
    const ShadowCascadeRenderer& shadow_renderer) const {
  // Set material uniforms
  fuselage_shader_.Use();
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "material.specular"), 0.7f, 0.7f, 0.7f);
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "material.shiny"), 1.0f);
  
  canopy_shader_.Use();
  glUniform3f(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "material.specular"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "material.shiny"), 64.0f);
  
  // Set common data
  std::vector<const Shader*> all_shaders = {&fuselage_shader_, &canopy_shader_,
    &exhaust_shader_};
  const glm::vec3& fog_color = sky.GetFogColor();
  const std::array<float,2>& fog_start_end = sky.GetFogStartEnd();
  for (const Shader* s : all_shaders) {
    s->Use();
    // Set view/projection uniforms
    glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "view"), 1, 
        GL_FALSE, glm::value_ptr(camera_.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "projection"), 1, 
        GL_FALSE, glm::value_ptr(camera_.GetProjectionMatrix()));

    // Set fog uniforms
    glUniform3f(glGetUniformLocation(s->GetProgram(), "fog.Color"),
        fog_color.x, fog_color.y, fog_color.z);
    glUniform1f(glGetUniformLocation(s->GetProgram(), "fog.Start"), 
        fog_start_end[0]);
    glUniform1f(glGetUniformLocation(s->GetProgram(), "fog.End"), 
        fog_start_end[1]);
    glUniform1i(glGetUniformLocation(s->GetProgram(), "fog.Equation"), 
        sky.GetFogEquation());
  }

  // Set data for aircraft
  std::vector<const Shader*> model_shaders = {&fuselage_shader_, 
    &canopy_shader_};
  const glm::vec3& sun_dir = sky.GetSunDirection();
  const glm::vec3& sun_color = sky.GetSunColor();
  for (const Shader* s : model_shaders) {
    s->Use();
    // Set lighting uniforms
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.direction"),
        sun_dir.x, sun_dir.y, sun_dir.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.ambient"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.diffuse"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.specular"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);

    // Set the camera position uniform
    glUniform3f(glGetUniformLocation(s->GetProgram(), "viewPos"), 
        0.0f, 0.0f, 0.0f);
  }

  // Set data for the engine flame
  glm::vec3 flame_color(1.0f, 0.76f, 0.44f);
  flame_color += 0.1*(float)rand()/(float)(RAND_MAX);
  fuselage_shader_.Use();
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame_color"), flame_color.x, flame_color.y, flame_color.z);
  
  glm::mat4 flame_model = GetAircraftModelMatrix();
  glm::vec3 flame1_pos = delta_flame_;
  glm::vec3 flame2_pos = delta_flame_;
  flame2_pos.x = -flame2_pos.x;
  glm::vec4 tmp = flame_model * glm::vec4(flame1_pos, 1.0f);
  tmp += 0.01*(float)rand()/(float)(RAND_MAX);
  flame1_pos = (glm::vec3)tmp;
  tmp = flame_model * glm::vec4(flame2_pos, 1.0f);
  tmp += 0.01*(float)rand()/(float)(RAND_MAX);
  flame2_pos = (glm::vec3)tmp;
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame1_pos"), flame1_pos.x, flame1_pos.y, flame1_pos.z);
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame2_pos"), flame2_pos.x, flame2_pos.y, flame2_pos.z);
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "r_flame"), r_flame_);
  GLfloat flame_alpha = std::pow(throttle_position_, 5.0); 
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame_alpha"), flame_alpha);      
  
  // Set the shadow data
  GLuint num_textures = model_.GetNumTextures();
  for (const Shader* s : model_shaders) {
    s->Use();
    glUniform1i(glGetUniformLocation(s->GetProgram(), "num_cascades"), 
        shadow_renderer.GetNumCascades());
    for (int i = 0; i < shadow_renderer.GetNumCascades(); ++i) { 
      // Send the depth maps
      glActiveTexture(GL_TEXTURE0 + num_textures + i);
      glBindTexture(GL_TEXTURE_2D, shadow_renderer.GetDepthMap(i));
      std::string tmp = "depthMap[" + std::to_string(i) + "]";
      glUniform1i(glGetUniformLocation(s->GetProgram(), tmp.c_str()), 
          num_textures + i);
      // Send the subfrusta end points
      tmp = "subfrusta_extents[" + std::to_string(i) + "]";
      glUniform1f(glGetUniformLocation(s->GetProgram(), tmp.c_str()), 
        shadow_renderer.GetSubfrustaExtent(i));
      // Send the light space matrices
      tmp = "lightSpaceMatrix[" + std::to_string(i) + "]";
      glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), tmp.c_str()), 
          1, GL_FALSE, glm::value_ptr(shadow_renderer.GetLightSpaceMatrix(i)));
      // Send the shadow biases
      tmp = "shadow_bias[" + std::to_string(i) + "]";
      glUniform1f(glGetUniformLocation(s->GetProgram(), tmp.c_str()), 
          shadow_renderer.GetShadowBias(i));
    }
    glm::vec3 camera_front = camera_.GetFront();
    glUniform3f(glGetUniformLocation(s->GetProgram(), "cameraFront"), 
        camera_front.x, camera_front.y, camera_front.z);
    glm::vec3 frustum_origin = camera_.GetFrustumOrigin();
    glUniform3f(glGetUniformLocation(s->GetProgram(), "frustumOrigin"), 
        frustum_origin.x, frustum_origin.y, frustum_origin.z);
    glm::vec3 frustum_terminus = camera_.GetFrustumTerminus();
    glUniform3f(glGetUniformLocation(s->GetProgram(), "frustumTerminus"), 
        frustum_terminus.x, frustum_terminus.y, frustum_terminus.z);
  }
}

//****************************************************************************80
void Aircraft::SetupDrawData() {
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;
  GLuint nstacks = 10;
  GLuint nslices = 10;
  float r = 1.0f;
  float sStep = M_PI / (float) nstacks;
  std::vector<float> t(nslices+1);
  t[0] = -M_PI/2;
  t[1] = t[0] + M_PI / 4;
  t.back() = M_PI/2;
  float dt = 2.0 * (M_PI/2 - (t[1] - t[0])) / (nslices - 2);
  for (GLuint i = 2; i < nslices; ++i) {
    t[i] = t[i-1] + dt;
  }
  for (GLuint i = 0; i < nslices; ++i) {
    for (float s = -M_PI; s <= M_PI-0.0001f; s += sStep) {
      float rt = r * (4.0f * t[i]*t[i]/M_PI/M_PI + 1.0f);
      vertices.push_back(rt * cos(t[i]) * cos(s));
      vertices.push_back(r * sin(t[i]));
      vertices.push_back(rt * cos(t[i]) * sin(s));

      rt = r * (4.0f * t[i+1]*t[i+1]/M_PI/M_PI + 1.0f);
      vertices.push_back(rt * cos(t[i+1]) * cos(s));
      vertices.push_back(r * sin(t[i+1]));
      vertices.push_back(rt * cos(t[i+1]) * sin(s));
    }
  }
  for (std::size_t i = 0; i < vertices.size()/3; i++) {
    indices.push_back((GLuint)i);
  }
  sphere_numindices_ = static_cast<GLuint>(indices.size());

  GLuint VBO, EBO;
  glGenVertexArrays(1, &sphere_VAO_);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(sphere_VAO_);

  // Set up the VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), 
      vertices.data(), GL_STATIC_DRAW);
  
  // Set up the EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), 
      indices.data(), GL_STATIC_DRAW);

  GLint pos_loc = glGetAttribLocation(exhaust_shader_.GetProgram(), 
      "position");
 
  // Position attribute
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 
      (GLvoid*)0);

  // Unbind VBO and VAO, but not EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
  glDeleteBuffers(1, &EBO); 
}

//****************************************************************************80
void Aircraft::DrawExhaust() const {
  exhaust_shader_.Use();

  // Enable face-culling 
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  
  // Orient model
  glm::mat4 exhaust_model = GetAircraftModelMatrix();

  // Set positions, transparency, etc.
  float xs = 0.433f; // width
  float ys = 1.0f; // length
  float zs = 0.210f; // height
  zs *= 0.6 + 0.4 * throttle_position_;
  std::vector<float> lengths = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
  std::vector<float> alphas = {0.3f, 0.2f, 0.1f, 0.05f, 0.02f};
  float tp0 = 0.6f; // throttle position where exhaust appears
  for (float& a : alphas) {
    if (throttle_position_ < tp0) 
      a *= 0.0f;
    else
      a *= (throttle_position_ - tp0) / (1.0f - tp0);
  }
  for (float& l : lengths) {
    if (throttle_position_ < tp0) 
      l *= 0.0f;
    else
      l *= std::pow(throttle_position_ - tp0, 1.0f/8.0f) /
        std::pow(1.0f - tp0, 1.0f/8.0f);
  }
  for (std::size_t i = 0; i < lengths.size(); ++i) {
    // Set exhaust color and transparency
    glm::vec3 color(0.878f, 0.597f, 0.6f);
    glm::vec3 flame_color(1.0f, 0.76f, 0.44f);
    float alpha = 0.5 + 0.5 * (float)i / (float)lengths.size();
    color = alpha * color + (1.0f - alpha) * flame_color;
    color.x += 0.1*(float)rand()/(float)(RAND_MAX);
    glUniform4f(glGetUniformLocation(exhaust_shader_.GetProgram(), 
          "exhaust_color"), color.x, color.y, color.z, alphas[i]);
 
    for (int e = 0; e < 2; ++e) {
      glm::vec3 delta_exhaust = delta_exhaust_;
      delta_exhaust.x *= -2.0f * (float)e + 1.0f;
      if (i > 0) {
        for (std::size_t ip = 0; ip < i; ++ip) {
          delta_exhaust.y -= 1.1*ys*lengths[ip];
        }
      }
      delta_exhaust += (float)i*0.01*(float)rand()/(float)(RAND_MAX);
      glm::mat4 model = glm::translate(exhaust_model, delta_exhaust);
      float xsl = xs*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      float ysl = ys*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      float zsl = zs*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      model = glm::scale(model, glm::vec3(xsl, ysl, zsl));
      
      // Set model uniform for exhaust
      glUniformMatrix4fv(glGetUniformLocation(exhaust_shader_.GetProgram(), 
            "model"), 1, GL_FALSE, glm::value_ptr(model));

      // Draw the ellipsoid
      glBindVertexArray(sphere_VAO_);
      glDrawElements(GL_TRIANGLE_STRIP, sphere_numindices_, GL_UNSIGNED_INT, 0);
    }
  }
  glBindVertexArray(0);

  // Disable face-culling
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
}

//****************************************************************************80
void Aircraft::UpdateEngineSounds() {
  engine_idle_.SetGain(0.2*(1.0 - throttle_position_));
  engine_idle_.SetPitch(std::min(1.2, 0.75 + 0.5*throttle_position_));
  afterburner_.SetGain(std::max(0.0, -0.5 + 2.0*throttle_position_));
}
  
//****************************************************************************80
std::vector<Aircraft::Contact> Aircraft::GetContacts(
    const std::vector<double>& state, float dt) const {
  glm::vec3 position = glm::vec3(state[0], state[1], state[2]);
  glm::quat orientation = glm::quat(state[3], state[4], state[5], state[6]);
  glm::vec3 velocity = glm::vec3(state[7], state[8], state[9]) * inv_mass_;
  glm::vec3 ang_momentum = glm::vec3(state[10], state[11], state[12]);
  
  glm::mat4 cm_model = glm::translate(glm::mat4(), position);
  cm_model = glm::translate(cm_model, delta_center_of_mass_);
  cm_model *= glm::toMat4(orientation);
  cm_model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
        glm::vec3(0.0f, 0.0f, 1.0f)));
  cm_model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)));
  cm_model = glm::translate(cm_model, -delta_center_of_mass_);
  glm::mat3 inv_inertia_w = glm::inverse(AircraftToWorld(inertia_, 
        orientation));
  
  std::vector<Contact> contacts;
  // Broad phase: determine if terrain AABB and aircraft AABB intersect
  bool broad_collide = false; // true if broad phase detects collision
  auto const& aabb_aircraft = model_.GetAABB();
  auto y_min_aircraft = position[1] + aabb_aircraft[1][0];
  for (int i = 0; i < 2 && !broad_collide; ++i) {
    for (int j = 0; j < 2 && !broad_collide; ++j) {
      auto xbb = position[0] + aabb_aircraft[0][i];
      auto zbb = position[2] + aabb_aircraft[2][j];
      auto y_max_terrain = terrain_.GetBoundingHeight(xbb, zbb);
      if (y_min_aircraft < y_max_terrain) {
        broad_collide = true;
      }
    }
  }
  if (!broad_collide)
    return contacts;

  // Narrow phase: check if any vertices of collision model are below terrain
  const float d_slop = 0.01; // penetration slop
  const float beta = 0.2; // error reduction parameter
  auto const& cm_verts = collision_model_.GetVertices(0);
  for (std::size_t i = 0; i < cm_verts.size(); ++i) {
    // Bring collision mesh vertex to world position
    auto cm_vert_w = glm::vec3(cm_model*glm::vec4(cm_verts[i].Position, 1.0));
    // Get the terrain height at this location
    auto y_terrain = terrain_.GetHeight(cm_vert_w[0], cm_vert_w[2]);
    auto n = terrain_.GetNormal(cm_vert_w[0], cm_vert_w[2]);
    float d = (y_terrain - cm_vert_w[1]) * n[1];
    if (d > 0.0f) {
      auto r = glm::vec3(cm_vert_w - position);
      auto v = velocity + glm::cross(inv_inertia_w * ang_momentum, r);
      auto v_dot_n = glm::dot(v,n); 
      auto t = glm::normalize(v - v_dot_n * n);
      auto mass_n = 1.0f / (inv_mass_ + 
          glm::dot(n, glm::cross(inv_inertia_w * glm::cross(r,n), r)));
      auto mass_t = 1.0f / (inv_mass_ + 
          glm::dot(t, glm::cross(inv_inertia_w * glm::cross(r,t), r)));
      // Add bias for position correction
      float bias = -beta / dt * std::min(0.0f, d_slop - d);
      // Add bias for bounce
      auto e = e_collision_;
      if (v_dot_n < 0.0f) {
        // Damp bounciness when object is "resting"
        if (-v_dot_n < 2.0 * 9.81 * dt * (1.0 + e * e))
          e = 0.0;
        bias -= e * v_dot_n;
      }
      contacts.push_back({d, n, t, v, r, mass_n, mass_t, bias});
    }
  }
  return contacts;
}

} // End namespace TopFun
