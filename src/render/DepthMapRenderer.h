#ifndef DEPTHMAPRENDERER_H
#define DEPTHMAPRENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TopFun {

class Terrain;
class Sky;
class Aircraft;
class Shader;
class Camera;

class DepthMapRenderer {
 public:
  DepthMapRenderer(GLfloat subfrustum_start, GLfloat subfrustum_end, 
      GLuint map_width, GLuint map_height);

  ~DepthMapRenderer() = default;

  inline GLuint GetDepthMap() const { return depth_map_; }
  
  inline const glm::mat4& GetLightSpaceMatrix() const { 
    return light_space_matrix_; 
  }

  void Render(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
      const Camera& camera, const glm::vec3& light_dir, 
      const Shader& shader); 

 private:
  GLfloat subfrustum_near_; // fractional distance between near and far planes
  GLfloat subfrustum_far_; // fractional distance between near and far planes
  GLuint map_width_;
  GLuint map_height_;
  GLuint depth_map_; // texture storing depth map
  GLuint depth_mapFBO_;
  glm::mat4 light_space_matrix_;

};
} // End namespace TopFun

#endif
