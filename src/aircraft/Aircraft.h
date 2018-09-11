#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "shaders/Shader.h"
#include "model/Model.h"
#include "render/Camera.h"
#include "audio/AudioSource.h"

namespace TopFun {

class ShadowCascadeRenderer;
class Sky;
class Terrain;

class Aircraft {
 
 public:
  //**************************************************************************80
  //! \brief Aircraft - Constructor
  //! \param[in] terrain - the terrain object containing heightmap data
  //! \param[in] camera - reference to camera
  //**************************************************************************80
  Aircraft(const glm::dvec3& position, const glm::quat& orientation,
      const Camera& camera, const Terrain& terrain);
  
  //**************************************************************************80
  //! \brief ~Aircraft - Destructor
  //**************************************************************************80
  ~Aircraft();

  //**************************************************************************80
  //! \brief Draw - draws the aircraft
  //**************************************************************************80
  void Draw(const Sky& sky, const ShadowCascadeRenderer* pshadow_renderer, 
      const Shader* shader=NULL);
  
  //**************************************************************************80
  //! \brief UpdateControls - process keyboard input to update ailerons, etc. 
  //**************************************************************************80
  void UpdateControls(std::vector<bool> const& keys);
  
  //**************************************************************************80
  //! \brief GetPosition - get the position vector
  //! returns - aircraft position vector
  //**************************************************************************80
  inline glm::dvec3 GetPosition() const { return position_; }
  
  //**************************************************************************80
  //! \brief GetVelocity - get the velocity vector
  //! returns - aircraft velocity vector
  //**************************************************************************80
  inline glm::vec3 GetVelocity() const { return lin_momentum_ * inv_mass_; }
  
  //**************************************************************************80
  //! \brief GetAngularVelocity - get the angular velocity vector
  //! returns - aircraft angular velocity vector in world coordinates
  //**************************************************************************80
  inline glm::vec3 GetAngularVelocity(const glm::quat& orientation,
      const glm::vec3& ang_momentum) const {
    return glm::inverse(AircraftToWorld(inertia_, orientation)) * ang_momentum;
  }
  
  //**************************************************************************80
  //! \brief GetAlpha - get the angle of attach
  //! returns - aircraft angle of attack (radians)
  //**************************************************************************80
  inline float GetAlpha() const { 
    return CalcAlpha(WorldToAircraft(lin_momentum_, orientation_) * inv_mass_); 
  }
  
  //**************************************************************************80
  //! \brief GetThrottlePosition - get the throttle position
  //! returns - throttle position
  //**************************************************************************80
  inline float GetThrottlePosition() const { return throttle_position_; }
  
  //**************************************************************************80
  //! \brief GetFrontDirection - get a vector pointing in the +x direction
  //! returns - aircraft front vector
  //**************************************************************************80
  inline glm::vec3 GetFrontDirection() const { 
    return AircraftToWorld(glm::vec3(1.0f, 0.0f, 0.0f), orientation_); 
  }
  
  //**************************************************************************80
  //! \brief GetUpDirection - get a vector pointing in the -z direction
  //! returns - aircraft up vector
  //**************************************************************************80
  inline glm::vec3 GetUpDirection() const { 
    return AircraftToWorld(glm::vec3(0.0f, 0.0f, -1.0f), orientation_); 
  }
  
  //**************************************************************************80
  //! \brief GetDeltaCenterOfMass - get the vector from the model center to
  //! the center of mass
  //! returns - vector from model center to CM
  //**************************************************************************80
  inline glm::dvec3 GetDeltaCenterOfMass() const { 
    return delta_center_of_mass_; 
  }

  //**************************************************************************80
  //! \brief GetState - get the position/orientation/momentum state vector
  //! returns - aircraft state vector
  //**************************************************************************80
  inline std::vector<double> GetState() {
    std::vector<double> state(13);
    for (int i = 0; i < 3; ++i) 
      state[i] = position_[i];
    state[3] = (double)orientation_.w;
    state[4] = (double)orientation_.x;
    state[5] = (double)orientation_.y;
    state[6] = (double)orientation_.z;
    for (int i = 0; i < 3; ++i) 
      state[i+7] = (double)lin_momentum_[i];
    for (int i = 0; i < 3; ++i) 
      state[i+10] = (double)ang_momentum_[i];
    return state;
  }

  //**************************************************************************80
  //! \brief SetState - set the position/orientation/momentum state vector
  //! param[in] state - aircraft state vector
  //**************************************************************************80
  inline void SetState(const std::vector<double>& state) {
    for (int i = 0; i < 3; ++i) 
      position_[i] = state[i];
    orientation_.w = (float)state[3];
    orientation_.x = (float)state[4];
    orientation_.y = (float)state[5];
    orientation_.z = (float)state[6];
    for (int i = 0; i < 3; ++i) 
      lin_momentum_[i] = (float)state[i+7];
    for (int i = 0; i < 3; ++i) 
      ang_momentum_[i] = (float)state[i+10];
    orientation_ = normalize(orientation_);

    // Update the audio source positions/velocities
    glm::mat4 model = glm::translate(glm::mat4(), (glm::vec3)position_);
    model = glm::translate(model, delta_center_of_mass_);
    model *= glm::toMat4(orientation_);
    model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
          glm::vec3(0.0f, 0.0f, 1.0f)));
    model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
          glm::vec3(1.0f, 0.0f, 0.0f)));
    model = glm::translate(model, -delta_center_of_mass_);
    glm::vec3 tmp(0.0, delta_flame_.y, delta_flame_.z);
    glm::vec4 sound_pos = model * glm::vec4(tmp, 1.0f);    
    engine_idle_.SetPosition((glm::vec3)sound_pos);
    afterburner_.SetPosition((glm::vec3)sound_pos);
    engine_idle_.SetVelocity(GetVelocity());
    afterburner_.SetVelocity(GetVelocity());
  }
  
  //**************************************************************************80
  //! \brief InterpolateState - interpolate state between timesteps
  //**************************************************************************80
  inline std::vector<double> InterpolateState(
      const std::vector<double>& previous_state, 
      const std::vector<double>& current_state, float alpha) const {
    std::vector<double> state_out(13);
    // Interpolate the position
    for (int i = 0; i < 3; ++i) 
      state_out[i] = alpha * current_state[i] 
        + (1.0f - alpha) * previous_state[i];
    // Slerp the orientation
    glm::quat co, po;
    co.w = current_state[3];
    co.x = current_state[4];
    co.y = current_state[5];
    co.z = current_state[6];
    po.w = previous_state[3];
    po.x = previous_state[4];
    po.y = previous_state[5];
    po.z = previous_state[6];
    glm::quat tmp = glm::slerp(co, po, alpha);
    state_out[3] = tmp.w;
    state_out[4] = tmp.x;
    state_out[5] = tmp.y;
    state_out[6] = tmp.z;
    // Interpolate the momentum
    for (int i = 7; i < 13; ++i) 
      state_out[i] = alpha * current_state[i] 
        + (1.0f - alpha) * previous_state[i];
    return state_out;
  }
  
  //**************************************************************************80
  //! \brief GetStateDerivative - evaluate the derivative of the state vector
  //! \param[in] state - current state vector
  //! \param[in] t - the current time
  //! \returns deriv - the derivative of the state vector
  //**************************************************************************80
  std::vector<double> GetStateDerivative(const std::vector<double>& state, 
      float t);
  
  //**************************************************************************80
  //! \brief DoPhysicsStep - perform integration of accelerations/velocities,
  //! update velocities/positions and resolve terrain collisions
  //! \param[in] t - the current time
  //! \param[in] dt - physics timestep
  //**************************************************************************80
  void DoPhysicsStep(float t, float dt);

 private:
  const Camera& camera_;
  const Terrain& terrain_;
  Shader fuselage_shader_;
  Shader canopy_shader_;
  Shader exhaust_shader_;
  Model model_;
  Model collision_model_;
  AudioSource engine_idle_;
  AudioSource afterburner_;

  // Primary state variables (all in world frame)
  glm::dvec3 position_; // world space absolution position
  glm::quat orientation_; // composition of all applied rotations 
  glm::vec3 lin_momentum_; 
  glm::vec3 ang_momentum_;

  // Secondary state variables (all in world frame)
  glm::vec3 forces_;
  glm::vec3 torques_;
  glm::vec3 acceleration_; 
  bool crashed_;

  // Control inputs
  float rudder_position_;
  float elevator_position_;
  float aileron_position_;
  const float rudder_position_max_ = 0.5f;
  const float elevator_position_max_ = 0.4f;
  const float aileron_position_max_ = 0.5f;
  float throttle_position_; // between 0.0 and 1.0
  int joystick_id_;

  // Longitudinal coefficients
  std::vector<float> CL_; // lift coefficient vs alpha
  std::vector<float> CD_; // drag coefficient vs alpha
  std::vector<float> Cm_; // moment coefficient vs alpha
  float CL_Q_; // lift due to pitch rate
  float Cm_Q_; // moment due to pitch rate
  float CL_alpha_dot_; // lift due to alpha rate
  float Cm_alpha_dot_; // moment due to alpha rate
  float CDi_CL2_; // induced drag coefficient (1/(pi*e*AR))

  // Lateral coefficients
  float CY_beta_; // side force due to sideslip
  float Cl_beta_; // dihedral effect
  float Cl_P_; // roll damping
  float Cl_R_; // roll due to yaw rate
  float Cn_beta_; // weather cocking stability
  float Cn_P_; // rudder adverse yaw
  float Cn_R_; // yaw damping

  // Control coefficients
  float CL_de_; // lift due to elevator
  float CD_de_; // drag due to elevator
  float CY_dr_; // side force due to rudder
  float Cm_de_; // pitch due to elevator
  float Cl_da_; // roll due to aileron 
  float Cn_da_; // yaw due to aileron
  float Cl_dr_; // roll due to rudder
  float Cn_dr_; // yaw due to rudder

  // Mass/Inertia/Dimensions/etc.
  float mass_;
  float inv_mass_;
  glm::vec3 delta_center_of_mass_; // from model origin
  glm::mat3 inertia_; // rotational inertia tensor
  float e_collision_; // coefficient of restitution
  float mu_static_; // coefficient of static friction
  float mu_dynamic_; // coefficient of dynamic friction
  float wetted_area_;
  float chord_;
  float span_;
  float dx_cg_x_ax_; // % chord from CG to aerodynamic center
  glm::vec3 r_tail_; // vector from center of mass to tail
  float max_thrust_;

  // Rotation axes for control surfaces 
  // First vector points to "base" of axis from origin
  // Second vector points in the axis direction
  std::array<glm::vec3,2> rudder_axis_;
  std::array<glm::vec3,2> aileron_axis_;
  std::array<glm::vec3,2> elevator_axis_;

  // Mesh indices for control surfaces
  std::vector<int> rudder_mesh_indices_;
  std::vector<int> aileron_mesh_indices_;
  std::vector<int> elevator_mesh_indices_;
  std::vector<int> airframe_mesh_indices_; // non-control surface components

  // Data for drawing engine exhaust
  GLuint sphere_VAO_;
  GLuint sphere_numindices_;
  glm::vec3 delta_exhaust_; // from model origin
  glm::vec3 delta_flame_; // from model origin
  GLfloat r_flame_; // only light faces within this radius
  
  //**************************************************************************80
  //! \brief WorldToAircraft - convert a vector from world coordinates to 
  //! aircraft local coordinates
  //! \details Aircraft is oriented like this:
  /*!     ^ x (front)
   *      |
   *      O 
   *  ====O====->
   *      O\    y (right)
   *      V \
   *     =|= v z (down)
   */    
  //! \param[in] world_vec - vector in world frame
  //! \param[in] orientation - orientation of aircraft
  //! \returns vector in aircraft frame
  //**************************************************************************80
  inline glm::vec3 WorldToAircraft(const glm::vec3& world_vec,
      const glm::quat& orientation) const {
    glm::quat world_quat = glm::quat(0.0f, world_vec);
    glm::quat result = glm::conjugate(orientation)*world_quat*orientation;
    return glm::vec3(result.x, result.y, result.z);
  }
  
  //**************************************************************************80
  //! \brief AircraftToWorld - convert a vector from aircraft coordinates to 
  //! world coordinates
  //! \param[in] aircraft_vec - vector in aircraft frame
  //! \param[in] orientation - orientation of aircraft
  //! \returns vector in world frame
  //**************************************************************************80
  inline glm::vec3 AircraftToWorld(const glm::vec3& aircraft_vec,
      const glm::quat& orientation) const {
    glm::quat aircraft_quat = glm::quat(0.0f, aircraft_vec);
    glm::quat result = orientation*aircraft_quat*glm::conjugate(orientation);
    return glm::vec3(result.x, result.y, result.z);
  }
  
  //**************************************************************************80
  //! \brief AircraftToWorld - convert a matrix from aircraft coordinates to 
  //! world coordinates
  //! \param[in] aircraft_mat - matrix in aircraft frame
  //! \param[in] orientation - orientation of aircraft
  //! \returns matrix in world frame
  //**************************************************************************80
  inline glm::mat3 AircraftToWorld(const glm::mat3& aircraft_mat,
      const glm::quat& orientation) const {
    glm::mat3 rot = glm::toMat3(orientation);
    return rot * aircraft_mat * glm::transpose(rot); 
  }

  //**************************************************************************80
  //! \brief CalcAlpha - calculate the angle of attack from velocity
  //! \param[in] v - velocity in aircraft frame
  //**************************************************************************80
  inline float CalcAlpha(const glm::vec3& v) const {
    if (std::abs(v.x) < std::numeric_limits<float>::epsilon()) {
      return 0.0f;
    }
    else {
      return atan(v.z / v.x);
    }
  } 
  
  //**************************************************************************80
  //! \brief CalcAlphaDot - calculate the time derivative of angle of attack
  //! \param[in] v - velocity in aircraft frame
  //! \param[in] a - acceleration in aircraft frame
  //**************************************************************************80
  inline float CalcAlphaDot(const glm::vec3& v, const glm::vec3& a) const {
    if (v.x*v.x + v.z*v.z < std::numeric_limits<float>::epsilon()) {
      return 0.0f;
    }
    else {
      return (v.x*a.z - v.z*a.x) / (v.x*v.x + v.z*v.z); 
    }
  } 
  
  //**************************************************************************80
  //! \brief CalcBeta - calculate the sideslip angle from velocity
  //! \param[in] v - velocity in aircraft frame
  //**************************************************************************80
  inline float CalcBeta(const glm::vec3& v) const {
    if (std::abs(v.x*v.x + v.z*v.z) < std::numeric_limits<float>::epsilon()) {
      return 0.0f;
    }
    else {
      return atan(v.y / std::sqrt(v.x*v.x + v.z*v.z));
    }
  } 
  
  //**************************************************************************80
  //! \brief CalcBetaDot - calculate the time derivative of sideslip angle
  //! \param[in] v - velocity in aircraft frame
  //! \param[in] a - acceleration in aircraft frame
  //**************************************************************************80
  inline float CalcBetaDot(const glm::vec3& v, const glm::vec3& a) const {
    float u2w2 = std::sqrt(v.x*v.x + v.z*v.z);
    if (std::abs(u2w2*(v.x*v.x + v.y*v.y + v.z*v.z)) < 
        std::numeric_limits<float>::epsilon()) {
      return 0.0f;
    }
    else {
      return (a.y*u2w2 - v.y*(v.x*a.x + v.z*a.z)) / 
        (u2w2*(v.x*v.x + v.y*v.y + v.z*v.z));
    }
  } 
  
  //**************************************************************************80
  //! \brief InterpolateAeroCoefficient - interpolate an aerodynamic coefficient
  //! for a given angle of attack
  //! \param[in] alpha - angle of attack (-pi < alpha < pi)
  //! \param[in] C - reference to the coefficient to be interpolated
  //! \returns - value of aerodynamic coefficient  
  //**************************************************************************80
  inline float InterpolateAeroCoefficient(float alpha, 
      const std::vector<float>& C) const {
    float dalpha = 2 * M_PI / (C.size() - 1);
    alpha += M_PI;
    size_t ix = static_cast<size_t>(std::floor(alpha / dalpha));
    return C[ix] + (C[ix+1] - C[ix]) / dalpha * (alpha - dalpha*ix);
  }
  
  //**************************************************************************80
  //! \brief CalcTailVelocity - calculate the wind velocity at the tail due to 
  //! aircraft rotation
  //! \param[in] omega - angular velocity in aircraft frame
  //**************************************************************************80
  inline float CalcTailVelocity(const glm::vec3& omega) const {
    return glm::l2Norm(glm::cross(omega, r_tail_));
  } 
  
  //**************************************************************************80
  //! \brief CalcLift - calculate the lift force (in aircraft frame)
  //! \param[in] alpha - angle of attack
  //! \param[in] alpha_dot - time derivative of angle of attack
  //! \param[in] omega - angular velocity in aircraft frame
  //! \param[in] vt - total velocity
  //! \param[in] dve - velocity across tail control surfaces 
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] de - elevator position
  //! \returns - value of lift
  //**************************************************************************80
  inline float CalcLift(float alpha, float alpha_dot, const glm::vec3& omega, 
      float vt, float dve, float q, float de) const {
    // Calculate the total lift coefficient
    float CL = InterpolateAeroCoefficient(alpha, CL_) + 
      (CL_Q_*omega.y + CL_alpha_dot_*alpha_dot)*chord_/2/vt + 
      CL_de_*de*(vt + dve)*(vt + dve)/vt/vt;
    return q*wetted_area_*CL;
  }

  //**************************************************************************80
  //! \brief CalcDrag - calculate the drag force (in aircraft frame)
  //! \param[in] lift - value of lift
  //! \param[in] alpha - angle of attack
  //! \param[in] vt - total velocity
  //! \param[in] dve - velocity across tail control surfaces 
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] de - elevator position
  //! \returns - value of drag
  //**************************************************************************80
  inline float CalcDrag(float lift, float alpha, float vt, float dve, float q, 
      float de) 
    const {
    // Calculate the total drag coefficient
    float CL = lift / q / wetted_area_;
    float CDt = InterpolateAeroCoefficient(alpha, CD_) + CL*CL*CDi_CL2_ 
      + CD_de_*std::abs(de)*(vt + dve)*(vt + dve)/vt/vt;
    return q*wetted_area_*CDt;
  }
  
  //**************************************************************************80
  //! \brief CalcSideForce - calculate the side force (in aircraft frame)
  //! \param[in] beta - sideslip angle
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] dr - rudder position
  //! \returns - value of side force
  //**************************************************************************80
  inline float CalcSideForce(float beta, float q, float dr) const {
    // Calculate the total side force coefficient
    float CYt = CY_beta_*beta + CY_dr_*dr;
    return q*wetted_area_*CYt;
  }
  
  //**************************************************************************80
  //! \brief CalcRollMoment - calculate the roll moment (in aircraft frame)
  //! \param[in] beta - sideslip angle
  //! \param[in] omega - angular velocity in aircraft frame
  //! \param[in] vt - total velocity
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] da - aileron position
  //! \param[in] dr - rudder position
  //! \returns - value of roll moment
  //**************************************************************************80
  inline float CalcRollMoment(float beta, const glm::vec3& omega, 
      float vt, float q, float da, float dr) const {
    // Calculate the total roll coefficient
    float Cl = (Cl_beta_*beta + (Cl_P_*omega.x + Cl_R_*omega.z)*span_/2/vt 
        + Cl_da_*da + Cl_dr_*dr);
    return q*wetted_area_*span_*Cl;
  }
  
  //**************************************************************************80
  //! \brief CalcPitchMoment - calculate the pitch moment (in aircraft frame)
  //! \param[in] alpha - angle of attack
  //! \param[in] alpha_dot - time derivative of angle of attack
  //! \param[in] omega - angular velocity in aircraft frame
  //! \param[in] vt - total velocity
  //! \param[in] dve - velocity across tail control surfaces 
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] de - elevator position
  //! \param[in] lift - lift force
  //! \param[in] drag - drag force
  //! \returns - value of pitch moment
  //**************************************************************************80
  inline float CalcPitchMoment(float alpha, float alpha_dot, 
      const glm::vec3& omega, float vt, float dve, float q, float de, 
      float lift, float drag) const {
    // Calculate the total pitch coefficient
    float Cm = InterpolateAeroCoefficient(alpha, Cm_) + (Cm_Q_*omega.y + 
        Cm_alpha_dot_*alpha_dot)*chord_/2/vt  
      + Cm_de_*de*(vt + dve)*(vt + dve)/vt/vt;
    float M_LD = dx_cg_x_ax_ * chord_ * (lift*cos(alpha) + drag*sin(alpha));
    return q*wetted_area_*chord_*Cm + M_LD;
  }
  
  //**************************************************************************80
  //! \brief CalcYawMoment - calculate the yaw moment (in aircraft frame)
  //! \param[in] beta - sideslip angle
  //! \param[in] omega - angular velocity in aircraft frame
  //! \param[in] vt - total velocity
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \param[in] da - aileron position
  //! \param[in] dr - rudder position
  //! \returns - value of yaw moment
  //**************************************************************************80
  inline float CalcYawMoment(float beta, const glm::vec3& omega, 
      float vt, float q, float da, float dr) const {
    // Calculate the total yaw coefficient
    float Cn = (Cn_beta_*beta + (Cn_P_*omega.x + Cn_R_*omega.z)*span_/2/vt 
        + Cn_da_*da + Cn_dr_*dr);
    return q*wetted_area_*span_*Cn;
  }
  
  //**************************************************************************80
  //! \brief CalcAeroForcesAndTorques - calculate all aerodynamic forces and
  //! torques acting on the aircraft (in aircraft frame)
  //! TODO
  //! \param[in] omega - angular velocity in aircraft frame
  //**************************************************************************80
  void CalcAeroForcesAndTorques(const glm::vec3& position,
      const glm::quat& orientation, const glm::vec3& lin_momentum, 
      const glm::vec3& omega, glm::vec3& forces, 
      glm::vec3& torques) const;

  //**************************************************************************80
  //! \brief CalcEngineForce - calculates the force vector due to the engine
  //! in the frame of the aircraft
  //! returns - engine thrust vector
  //**************************************************************************80
  inline glm::vec3 CalcEngineForce() const {
    return glm::vec3(max_thrust_ * throttle_position_, 0.0f, 0.0f);
  }
  
  //**************************************************************************80
  //! \brief CalcGravityForce - calculates the force vector due to gravity in
  //! the world frame
  //! returns - gravity force vector
  //**************************************************************************80
  inline glm::vec3 CalcGravityForce() const {
    return glm::vec3(0.0f, -mass_ * 9.81f, 0.0f);
  }
  
  //**************************************************************************80
  //! \brief GetAircraftModelMatrix - get the model matrix for the aircraft
  //**************************************************************************80
  inline glm::mat4 GetAircraftModelMatrix() const {
    // Translate model to current position
    glm::mat4 aircraft_model = glm::translate(glm::mat4(), 
        (glm::vec3)(position_ - camera_.GetPosition()));
    aircraft_model = glm::translate(aircraft_model, delta_center_of_mass_);
    // Rotate model to current orientation
    aircraft_model *= glm::toMat4(orientation_);
    // Rotate model to align with aircraft axis definition
    aircraft_model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
          glm::vec3(0.0f, 0.0f, 1.0f)));
    aircraft_model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
          glm::vec3(1.0f, 0.0f, 0.0f)));
    aircraft_model = glm::translate(aircraft_model, -delta_center_of_mass_);
    return aircraft_model;
  }
  
  //**************************************************************************80
  //! \brief GetControlSurfaceModelMatrix - get model matrix for control surface
  //! \param[in] displacement - vector from model origin to rotation axis 
  //! \param[in] axis - axis to rotate around 
  //! \param[in] deflection_angle - angle to rotate control surface (radians)
  //! \param[in] right - true if right control surface
  //**************************************************************************80
  inline glm::mat4 GetControlSurfaceModelMatrix(glm::vec3 displacement, 
      glm::vec3 axis, float deflection_angle, 
      bool right=false) const {
    if (right) {
      displacement.x = -displacement.x;
      axis.x = -axis.x;
    }
    // Translate model to aircraft model origin
    glm::mat4 model = glm::translate(glm::mat4(), displacement);
    // Rotate the model around its axis
    model *= glm::toMat4(glm::angleAxis(deflection_angle, axis));
    // Translate model back to original position
    model = glm::translate(model, -displacement);
    // Apply the model matrix of the airframe
    model = GetAircraftModelMatrix() * model;
    return model;
  }

  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(const Sky& sky,
      const ShadowCascadeRenderer& shadow_renderer) const;
  
  //**************************************************************************80
  //! \brief SetupDrawData - helper for setting up data to draw
  //**************************************************************************80
  void SetupDrawData();
  
  //**************************************************************************80
  //! \brief DrawExhaust - draw the engine exhaust
  //**************************************************************************80
  void DrawExhaust() const;
  
  //**************************************************************************80
  //! \brief UpdateEngineSounds - update the engine sounds based on throttle
  //**************************************************************************80
  void UpdateEngineSounds();

  struct Contact {
    float d; // penetration amount
    glm::vec3 n; // contact normal
    glm::vec3 t; // contact tangent
    glm::vec3 v; // contact velocity
    glm::vec3 r; // vector from CM to contact point
    float mass_n; // normal mass
    float mass_t; // tangent mass
    float bias; // velocity bias
    float j_n; // accumulated normal impulse
    float j_t; // accumulated tangent impulse
  };

  //**************************************************************************80
  //! \brief GetContacts - get the set of contacts
  //! param[in] state - state vector
  //! \param[in] dt - physics timestep
  //! \returns vector of contact points
  //**************************************************************************80
  std::vector<Contact> GetContacts(const std::vector<double>& state,
      float dt) const;

};
} // End namespace TopFun

#endif
