#ifndef DEPTHMAPRENDERER_H
#define DEPTHMAPRENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "render/Camera.h"

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
      const Camera& camera, const glm::vec3& light_dir); 

  inline void ToggleVisible() {
    visible_ = !visible_;
  }
  
  void Display(); 

 private:
  GLuint map_width_;
  GLuint map_height_;
  Shader shader_; // depth map shader
  Shader debug_shader_; // shader to display depth map texture
  bool visible_; // controls if texture is rendered for debugging
  GLuint quadVAO_; // for rendering depth map texture
  GLuint quadVBO_; // for rendering depth map texture
  GLuint depth_map_; // texture storing depth map
  GLuint depth_mapFBO_;
  glm::mat4 light_space_matrix_;
  const GLfloat scale_factor_ = 0.01f; // factor of camera view distance

};
} // End namespace TopFun

#endif
