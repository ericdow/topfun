#ifndef SHADOWRENDERER_H
#define SHADOWRENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"

namespace TopFun {

class ShadowRenderer {
 public:
  ShadowRenderer(GLuint shadow_width, GLuint shadow_height);

  ~ShadowRenderer() = default;

  inline GLuint GetDepthMap() { return depth_map_; }

  void RenderDepthMap(const glm::vec3& light_pos);

 private:
  GLuint shadow_width_;
  GLuint shadow_height_;
  Shader shader_;
  GLuint depth_map_;
  GLuint depth_mapFBO_;

};
} // End namespace TopFun

#endif
