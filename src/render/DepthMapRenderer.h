#ifndef DEPTHMAPRENDERER_H
#define DEPTHMAPRENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "utils/Camera.h"

namespace TopFun {

class Terrain;
class Sky;
class Aircraft;

class DepthMapRenderer {
 public:
  DepthMapRenderer(GLuint map_width, GLuint map_height);

  ~DepthMapRenderer() = default;

  inline GLuint GetDepthMap() const { return depth_map_; }
  
  inline const glm::mat4& GetLightSpaceMatrix() const { 
    return light_space_matrix_; 
  }

  void Render(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
      const Camera& camera, const glm::vec3& light_pos, 
      const glm::vec3& scene_center);

 private:
  GLuint map_width_;
  GLuint map_height_;
  Shader shader_; // depth map shader
  GLuint depth_map_; // texture storing depth map
  GLuint depth_mapFBO_;
  glm::mat4 light_space_matrix_;

};
} // End namespace TopFun

#endif
