#include "geometry/BoundingFrustum.h"
#include "geometry/BoundingBox.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
BoundingFrustum::BoundingFrustum(const glm::mat4& viewMat, 
    const glm::mat4& projectionMat) : BoundingVolume() {
  const glm::mat4& v = viewMat;
  const glm::mat4& p = projectionMat;

  // Build the clip matrix
  glm::mat4 cm;

  cm[0][0] = v[0][0]*p[0][0]+v[0][1]*p[1][0]+v[0][2]*p[2][0]+v[0][3]*p[3][0];
  cm[1][0] = v[0][0]*p[0][1]+v[0][1]*p[1][1]+v[0][2]*p[2][1]+v[0][3]*p[3][1];
  cm[2][0] = v[0][0]*p[0][2]+v[0][1]*p[1][2]+v[0][2]*p[2][2]+v[0][3]*p[3][2];
  cm[3][0] = v[0][0]*p[0][3]+v[0][1]*p[1][3]+v[0][2]*p[2][3]+v[0][3]*p[3][3];
  cm[0][1] = v[1][0]*p[0][0]+v[1][1]*p[1][0]+v[1][2]*p[2][0]+v[1][3]*p[3][0];
  cm[1][1] = v[1][0]*p[0][1]+v[1][1]*p[1][1]+v[1][2]*p[2][1]+v[1][3]*p[3][1];
  cm[2][1] = v[1][0]*p[0][2]+v[1][1]*p[1][2]+v[1][2]*p[2][2]+v[1][3]*p[3][2];
  cm[3][1] = v[1][0]*p[0][3]+v[1][1]*p[1][3]+v[1][2]*p[2][3]+v[1][3]*p[3][3];
  cm[0][2] = v[2][0]*p[0][0]+v[2][1]*p[1][0]+v[2][2]*p[2][0]+v[2][3]*p[3][0];
  cm[1][2] = v[2][0]*p[0][1]+v[2][1]*p[1][1]+v[2][2]*p[2][1]+v[2][3]*p[3][1];
  cm[2][2] = v[2][0]*p[0][2]+v[2][1]*p[1][2]+v[2][2]*p[2][2]+v[2][3]*p[3][2];
  cm[3][2] = v[2][0]*p[0][3]+v[2][1]*p[1][3]+v[2][2]*p[2][3]+v[2][3]*p[3][3];
  cm[0][3] = v[3][0]*p[0][0]+v[3][1]*p[1][0]+v[3][2]*p[2][0]+v[3][3]*p[3][0];
  cm[1][3] = v[3][0]*p[0][1]+v[3][1]*p[1][1]+v[3][2]*p[2][1]+v[3][3]*p[3][1];
  cm[2][3] = v[3][0]*p[0][2]+v[3][1]*p[1][2]+v[3][2]*p[2][2]+v[3][3]*p[3][2];
  cm[3][3] = v[3][0]*p[0][3]+v[3][1]*p[1][3]+v[3][2]*p[2][3]+v[3][3]*p[3][3];

  m_planes_[RIGHT].x = cm[3][0]-cm[0][0];
  m_planes_[RIGHT].y = cm[3][1]-cm[0][1];
  m_planes_[RIGHT].z = cm[3][2]-cm[0][2];
  m_planes_[RIGHT].w = cm[3][3]-cm[0][3];

  m_planes_[LEFT].x = cm[3][0]+cm[0][0];
  m_planes_[LEFT].y = cm[3][1]+cm[0][1];
  m_planes_[LEFT].z = cm[3][2]+cm[0][2];
  m_planes_[LEFT].w = cm[3][3]+cm[0][3];

  m_planes_[BOTTOM].x = cm[3][0]+cm[1][0];
  m_planes_[BOTTOM].y = cm[3][1]+cm[1][1];
  m_planes_[BOTTOM].z = cm[3][2]+cm[1][2];
  m_planes_[BOTTOM].w = cm[3][3]+cm[1][3];

  m_planes_[TOP].x = cm[3][0]-cm[1][0];
  m_planes_[TOP].y = cm[3][1]-cm[1][1];
  m_planes_[TOP].z = cm[3][2]-cm[1][2];
  m_planes_[TOP].w = cm[3][3]-cm[1][3];

  m_planes_[BACK].x = cm[3][0]-cm[2][0];
  m_planes_[BACK].y = cm[3][1]-cm[2][1];
  m_planes_[BACK].z = cm[3][2]-cm[2][2];
  m_planes_[BACK].w = cm[3][3]-cm[2][3];

  m_planes_[FRONT].x = cm[3][0]+cm[2][0];
  m_planes_[FRONT].y = cm[3][1]+cm[2][1];
  m_planes_[FRONT].z = cm[3][2]+cm[2][2];
  m_planes_[FRONT].w = cm[3][3]+cm[2][3];

  for(int i = 0; i < 6; i++) {
    m_planes_[i] = glm::normalize(m_planes_[i]);
  }
}
 
//****************************************************************************80
BoundingVolume::TestResult BoundingFrustum::TestIntersection(
    const BoundingBox& box) const {
  TestResult result = INSIDE;
 
  for(int i = 0; i < 6; i++) {
    const float pos = m_planes_[i].w;
    const glm::vec3 normal = glm::vec3(m_planes_[i]);
 
    if (glm::dot(normal, box.GetPositiveVertex(normal))+pos < 0.0f) {
      return OUTSIDE;
    }
    if (glm::dot(normal, box.GetNegativeVertex(normal))+pos < 0.0f) {
      result = INTERSECT;
    }
  } 
  return result;
}

} // End namespace TopFun
