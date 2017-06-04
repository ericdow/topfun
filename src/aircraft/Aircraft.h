#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>

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

 private:
  Shader shader_;
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

  /* 
  // Longitudinal coefficients
  const std::vector<float> CL_; // lift coefficient vs AoA
  const std::vector<float> CD_; // drag coefficient vs AoA
  const std::vector<float> Cm_; // moment coefficient vs AoA
  const float CL_Q_; // lift due to pitch rate
  const float Cm_Q_; // moment due to pitch rate
  const float CL_alpha_dot_; // lift due to AoA rate
  const float Cm_alpha_dot_; // moment due to AoA rate

  // Lateral coefficients
  const float CY_beta_; // side force due to sideslip
  const float Cl_beta_; // dihedral effect
  const float Cl_P_; // roll damping
  const float Cl_R_; // roll due to yaw rate
  const float Cn_beta_; // weather cocking stability
  const float Cn_P_; // rudder adverse yaw
  const float Cn_R_; // yaw damping

  // Control coefficients
  const float CL_de_; // lift due to elevator
  const float CD_de_; // drag due to elevator
  const float CY_de_; // side force due to elevator
  const float Cm_de_; // pitch due to elevator
  const float Cl_da_; // roll due to aileron 
  const float Cn_da_; // yaw due to aileron
  const float Cl_dr_; // roll due to rudder
  const float Cn_dr_; // yaw due to rudder
  */
  
  //**************************************************************************80
  //! \brief WorldToAircraft - convert a vector from world coordinates to 
  //! aircraft local coordinates
  //**************************************************************************80
  inline glm::vec3 WorldToAircraft(const glm::vec3& world_vec) {
    glm::quat world_quat = glm::quat(0.0f, world_vec);
    glm::quat result = orientation_ * world_quat * glm::conjugate(orientation_);
    return glm::vec3(result.x, result.y, result.z);
  }

  //**************************************************************************80
  //! \brief CalcAoA - calculate the angle of attack from velocity
  //**************************************************************************80
  inline float CalcAoA() {
    glm::vec3 lin_momentum_rel = WorldToAircraft(lin_momentum_);
    return atan(lin_momentum_rel.z / lin_momentum_rel.x);
  } 
  
  //**************************************************************************80
  //! \brief CalcSideslip - calculate the sideslip angle from velocity
  //**************************************************************************80
  inline float CalcSideslip() {
    glm::vec3 lin_momentum_rel = WorldToAircraft(lin_momentum_);
    return atan(lin_momentum_rel.y / lin_momentum_rel.x);
  } 

  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera);

};
} // End namespace TopFun

#endif
