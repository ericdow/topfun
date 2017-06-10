#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "shaders/Shader.h"
#include "utils/Model.h"
#include "utils/Camera.h"

namespace TopFun {

class Aircraft {
 
 public:
  //**************************************************************************80
  //! \brief Aircraft - Constructor
  //**************************************************************************80
  Aircraft(const glm::vec3& position, const glm::quat& orientation);
  
  //**************************************************************************80
  //! \brief ~Aircraft - Destructor
  //**************************************************************************80
  ~Aircraft() = default;

  //**************************************************************************80
  //! \brief Draw - draws the aircraft
  //**************************************************************************80
  void Draw(Camera const& camera);
  
  //**************************************************************************80
  //! \brief Move - process keyboard input to move the aircraft
  //**************************************************************************80
  void Move(std::vector<bool> const& keys, float deltaTime);

 private:
  Shader fuselage_shader_;
  Shader canopy_shader_;
  Model model_;

  // Primary state variables (all in world frame)
  glm::vec3 position_;
  glm::quat orientation_; // composition of all applied rotations 
  glm::vec3 lin_momentum_; 
  glm::vec3 ang_momentum_; 

  // Control inputs
  float rudder_position_;
  float elevator_position_;
  float aileron_position_;
  float throttle_position_;

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

  // Mass/Inertia/etc.
  float mass_;
  glm::vec3 delta_center_of_mass_; // from model centroid
  glm::vec3 inertia_; // rotational inertia (I_xx, I_yy, I_zz)
  glm::vec3 inverse_inertia_;
  float wetted_area_;
  float chord_;
  glm::vec3 r_tail_; // vector from center of mass to tail
  
  //**************************************************************************80
  //! \brief Rotate - rotate the aircraft based on keyboard input
  //**************************************************************************80
  void Rotate(float angle, glm::vec3 axis);
  
  //**************************************************************************80
  //! \brief WorldToAircraft - convert a vector from world coordinates to 
  //! aircraft local coordinates
  //**************************************************************************80
  inline glm::vec3 WorldToAircraft(const glm::vec3& world_vec) const {
    glm::quat world_quat = glm::quat(0.0f, world_vec);
    glm::quat result = orientation_ * world_quat * glm::conjugate(orientation_);
    return glm::vec3(result.x, result.y, result.z);
  }

  //**************************************************************************80
  //! \brief CalcAlpha - calculate the angle of attack from velocity
  //! \param[in] v - velocity in aircraft frame
  //**************************************************************************80
  inline float CalcAlpha(const glm::vec3& v) const {
    return atan(v.z / v.x);
  } 
  
  //**************************************************************************80
  //! \brief CalcAlphaRate - calculate the time derivative of angle of attack
  //! \param[in] v - velocity in aircraft frame
  //! \param[in] a - acceleration in aircraft frame
  //**************************************************************************80
  inline float CalcAlphaRate(const glm::vec3& v, const glm::vec3& a) const {
    return (v.x*a.z - v.z*a.x) / std::sqrt(v.x*v.x + v.z*v.z); 
  } 
  
  //**************************************************************************80
  //! \brief CalcBeta - calculate the sideslip angle from velocity
  //! \param[in] v - velocity in aircraft frame
  //**************************************************************************80
  inline float CalcBeta(const glm::vec3& v) const {
    return atan(v.y / std::sqrt(v.x*v.x + v.z*v.z));
  } 
  
  //**************************************************************************80
  //! \brief CalcBetaRate - calculate the time derivative of sideslip angle
  //! \param[in] v - velocity in aircraft frame
  //! \param[in] a - acceleration in aircraft frame
  //**************************************************************************80
  inline float CalcBetaRate(const glm::vec3& v, const glm::vec3& a) const {
    float u2w2 = std::sqrt(v.x*v.x + v.z*v.z);
    return (a.y*u2w2 - v.y*(v.x*a.x + v.z*a.z)) / 
      (u2w2*(v.x*v.x + v.y*v.y + v.z*v.z));
  } 
  
  //**************************************************************************80
  //! \brief InterpolateAeroCoefficient - interpolate an aerodynamic coefficient
  //! for a given angle of attack
  //! \param[in] alpha - angle of attack
  //! \param[in] C - reference to the coefficient to be interpolated
  //! \returns - value of aerodynamic coefficient  
  //**************************************************************************80
  inline float InterpolateAeroCoefficient(float alpha, 
      const std::vector<float>& C) const {
    float dalpha = 2 * M_PI / (C.size() - 1);
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
  //! \returns - value of drag
  //**************************************************************************80
  inline float CalcLift(float alpha, float alpha_dot, const glm::vec3& omega, 
      float vt, float dve, float q) const {
    // Calculate the total lift coefficient
    float CL = InterpolateAeroCoefficient(alpha, CL_) + 
      (CL_Q_*omega.y + CL_alpha_dot_*alpha_dot)*chord_/2/vt + 
      CL_de_*elevator_position_*(vt + dve)*(vt + dve)/vt/vt;
    return q*wetted_area_*CL;
  }

  //**************************************************************************80
  //! \brief CalcDrag - calculate the drag force (in aircraft frame)
  //! \param[in] alpha - angle of attack
  //! \param[in] vt - total velocity
  //! \param[in] dve - velocity across tail control surfaces 
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \returns - value of drag
  //**************************************************************************80
  inline float CalcDrag(float alpha, float vt, float dve, float q) const {
    // Calculate the total drag coefficient
    float CL = InterpolateAeroCoefficient(alpha, CL_);
    float CDt = InterpolateAeroCoefficient(alpha, CD_) + CL*CL*CDi_CL2_ 
      + CD_de_*elevator_position_*(vt + dve)*(vt + dve)/vt/vt;
    return q*wetted_area_*CDt;
  }
  
  //**************************************************************************80
  //! \brief CalcSideForce - calculate the side force (in aircraft frame)
  //! \param[in] beta - sideslip angle
  //! \param[in] q - dynamic pressure (1/2 rho vt^2)
  //! \returns - value of side force
  //**************************************************************************80
  inline float CalcSideForce(float beta, float q) const {
    // Calculate the total side force coefficient
    float CYt = CY_beta_*beta + CY_dr_*rudder_position_;
    return q*wetted_area_*CYt;
  }

  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera);

};
} // End namespace TopFun

#endif
