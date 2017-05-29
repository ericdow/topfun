#ifndef BOUNDINGFRUSTRUM_H
#define BOUNDINGFRUSTRUM_H
#include <iostream>

#include "geometry/BoundingVolume.h"

namespace TopFun {

class BoundingFrustum : public BoundingVolume {
 public:

  BoundingFrustum(const glm::mat4& viewMatrix, 
      const glm::mat4& projectionMatrix);

  virtual ~BoundingFrustum() = default;

  enum Plane {
    BACK,
    FRONT,
    RIGHT,
    LEFT,
    TOP,
    BOTTOM
  };

  const glm::vec4& GetPlane(const int plane) const;
 
  TestResult TestIntersection(const glm::vec3& point) const;
  TestResult TestIntersection(const BoundingBox& box) const;
  TestResult TestIntersection(const BoundingSphere& sphere) const;

 private:
  glm::vec4 m_planes_[6];
};

} // End namespace TopFun

#endif
