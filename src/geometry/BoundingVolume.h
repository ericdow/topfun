#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include <glm/glm.hpp>

namespace TopFun {

class BoundingBox;
class BoundingSphere;

class BoundingVolume {
 public:

  virtual ~BoundingVolume() = default;

  enum TestResult {
    OUTSIDE,
    INTERSECT,
    INSIDE
  };

  virtual TestResult TestIntersection(const glm::vec3& point) const = 0;
  virtual TestResult TestIntersection(const BoundingBox& box) const = 0;
  virtual TestResult TestIntersection(const BoundingSphere& sphere) const = 0;
};

} // End namespace TopFun

#endif
