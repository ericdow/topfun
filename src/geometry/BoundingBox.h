#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H
#include <iostream>

#include "geometry/BoundingVolume.h"

namespace TopFun {

class BoundingBox : public BoundingVolume {
 public:

  BoundingBox(glm::vec3 minimum, glm::vec3 maximum, glm::vec3 position); 

  virtual ~BoundingBox() = default;

  virtual glm::vec3 GetPositiveVertex(const glm::vec3& normal) const;
  virtual glm::vec3 GetNegativeVertex(const glm::vec3& normal) const;

  TestResult TestIntersection(const glm::vec3& point) const;
  TestResult TestIntersection(const BoundingBox& box) const;
  TestResult TestIntersection(const BoundingSphere& sphere) const;

 private:
  
  glm::vec3 minimum_;
  glm::vec3 maximum_;
  glm::vec3 position_;

};

} // End namespace TopFun

#endif
