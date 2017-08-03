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

namespace TopFun {

class DepthMapRenderer;
class Sky;

class Aircraft {
 
 public:
  //**************************************************************************80
  //! \brief Aircraft - Constructor
  //**************************************************************************80
  Aircraft(const glm::vec3& position, const glm::quat& orientation);
  
  //**************************************************************************80
  //! \brief ~Aircraft - Destructor
  //**************************************************************************80
  ~Aircraft();

  //**************************************************************************80
  //! \brief Draw - draws the aircraft
  //**************************************************************************80
  void Draw(const Camera& camera, const Sky& sky, 
      const DepthMapRenderer& depthmap_renderer, const Shader* shader=NULL);
  
  //**************************************************************************80
  //! \brief UpdateControls - process keyboard input to update ailerons, etc. 
  //**************************************************************************80
  void UpdateControls(std::vector<bool> const& keys);
  
  //**************************************************************************80
  //! \brief GetPosition - get the position vector
  //! returns - aircraft position vector
  //**************************************************************************80
  inline glm::vec3 GetPosition() const { return position_; }
  
  //**************************************************************************80
  //! \brief GetVelocity - get the velocity vector
  //! returns - aircraft velocity vector
  //**************************************************************************80
  inline glm::vec3 GetVelocity() const { return lin_momentum_ / mass_; }
  
  //**************************************************************************80
  //! \brief GetAlpha - get the angle of attach
  //! returns - aircraft angle of attack (radians)
  //**************************************************************************80
  inline float GetAlpha() const { 
    return CalcAlpha(WorldToAircraft(lin_momentum_, orientation_) / mass_); 
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
  //! \brief GetState - get the position/orientation/momentum state vector
  //! returns - aircraft state vector
  //**************************************************************************80
  inline std::vector<float> GetState() {
    std::vector<float> state(13);
    for (int i = 0; i < 3; ++i) 
      state[i] = position_[i];
    state[3] = orientation_.w;
    state[4] = orientation_.x;
    state[5] = orientation_.y;
    state[6] = orientation_.z;
    for (int i = 0; i < 3; ++i) 
      state[i+7] = lin_momentum_[i];
    for (int i = 0; i < 3; ++i) 
      state[i+10] = ang_momentum_[i];
    return state;
  }

  //**************************************************************************80
  //! \brief SetState - set the position/orientation/momentum state vector
  //! param[in] state - aircraft state vector
  //**************************************************************************80
  inline void SetState(const std::vector<float>& state) {
    for (int i = 0; i < 3; ++i) 
      position_[i] = state[i];
    orientation_.w = state[3];
    orientation_.x = state[4];
    orientation_.y = state[5];
    orientation_.z = state[6];
    for (int i = 0; i < 3; ++i) 
      lin_momentum_[i] = state[i+7];
    for (int i = 0; i < 3; ++i) 
      ang_momentum_[i] = state[i+10];
    orientation_ = normalize(orientation_);
  }
  
  //**************************************************************************80
  //! \brief InterpolateState - interpolate state between timesteps
  //**************************************************************************80
  inline void InterpolateState(const std::vector<float>& previous_state, 
      std::vector<float>& current_state, float alpha) {
    for (int i = 0; i < 3; ++i) 
      current_state[i]  = alpha * current_state[i] 
        + (1.0f - alpha) * previous_state[i];
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
    current_state[3] = tmp.w;
    current_state[4] = tmp.x;
    current_state[5] = tmp.y;
    current_state[6] = tmp.z;
    for (int i = 7; i < 13; ++i) 
      current_state[i]  = alpha * current_state[i] 
        + (1.0f - alpha) * previous_state[i];
  }
  
  //**************************************************************************80
  //! \brief operator() - evaluate the derivative of the state vector
  //! \param[in] state - current state vector
  //! \param[out] deriv - the derivative of the state vector
  //! \param[in] t - the current time
  //**************************************************************************80
  void operator()(const std::vector<float>& state, std::vector<float>& deriv, 
      float t);

 private:
  Shader fuselage_shader_;
  Shader canopy_shader_;
  Shader exhaust_shader_;
  Model model_;

  // Primary state variables (all in world frame)
  glm::vec3 position_;
  glm::quat orientation_; // composition of all applied rotations 
  glm::vec3 lin_momentum_; 
  glm::vec3 ang_momentum_;

  // Secondary state variables (all in world frame)
  glm::vec3 acceleration_; 

  // Control inputs
  float rudder_position_;
  float elevator_position_;
  float aileron_position_;
  const float rudder_position_max_ = 0.2618f;
  const float elevator_position_max_ = 0.5236f;
  const float aileron_position_max_ = 0.5236f;
  float throttle_position_; // between 0.0 and 1.0

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
  glm::vec3 delta_center_of_mass_; // from model origin
  glm::mat3 inertia_; // rotational inertia tensor
  float wetted_area_;
  float chord_;
  float span_;
  float dx_cg_x_ax_; // % chord from CG to aerodynamic center
  glm::vec3 r_tail_; // vector from center of mass to tail
  float max_thrust_;

  // Rotation axes for control surfaces (axis is line segment connecting points)
  std::array<glm::vec3,2> rudder_axis_;
  std::array<glm::vec3,2> aileron_axis_;
  std::array<glm::vec3,2> elevator_axis_;

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
  //! \brief GetAircraftModel - get the model matrix for the aircraft
  //**************************************************************************80
  inline glm::mat4 GetAircraftModel() const {
    // Translate model to current position
    glm::mat4 aircraft_model = glm::translate(glm::mat4(), position_);
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
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera, const Sky& sky,
      const DepthMapRenderer& depthmap_renderer);
  
  //**************************************************************************80
  //! \brief SetupDrawData - helper for setting up data to draw
  //**************************************************************************80
  void SetupDrawData();
  
  //**************************************************************************80
  //! \brief DrawExhaust - draw the engine exhaust
  //**************************************************************************80
  void DrawExhaust();

};
} // End namespace TopFun

#endif
