#include "geometry/BoundingBox.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
BoundingBox::BoundingBox(glm::vec3 minimum, glm::vec3 maximum,
    glm::vec3 position = glm::vec3(0.0f)) : 
  BoundingVolume(), minimum_(minimum), maximum_(maximum), 
  position_(position) {}

//****************************************************************************80
glm::vec3 BoundingBox::GetPositiveVertex(const glm::vec3& normal) const {
  glm::vec3 positiveVertex = minimum_;

  if (normal.x >= 0.0f) positiveVertex.x = maximum_.x;
  if (normal.y >= 0.0f) positiveVertex.y = maximum_.y;
  if (normal.z >= 0.0f) positiveVertex.z = maximum_.z;

  return position_ + positiveVertex;
}
 
//****************************************************************************80
glm::vec3 BoundingBox::GetNegativeVertex(const glm::vec3& normal) const {
  glm::vec3 negativeVertex = maximum_;

  if (normal.x >= 0.0f) negativeVertex.x = minimum_.x;
  if (normal.y >= 0.0f) negativeVertex.y = minimum_.y;
  if (normal.z >= 0.0f) negativeVertex.z = minimum_.z;
 
  return position_ + negativeVertex;
}

} // End namespace TopFun
