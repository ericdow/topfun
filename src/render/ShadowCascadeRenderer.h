#ifndef SHADOWCASCADERENDERER_H
#define SHADOWCASCADERENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "render/Camera.h"
#include "render/DepthMapRenderer.h"

namespace TopFun {

class Terrain;
class Sky;
class Aircraft;

class ShadowCascadeRenderer {
 public:
  ShadowCascadeRenderer(GLuint map_width, GLuint map_height, 
      const std::vector<float>& subfrusta_extents,
      const std::vector<float>& shadow_biases);

  ~ShadowCascadeRenderer() = default;

  void Render(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
      const Camera& camera, const glm::vec3& light_dir);

  inline int GetNumCascades() const { return depth_map_renderers_.size(); }

  inline GLuint GetDepthMap(int i) const {
    return depth_map_renderers_[i].GetDepthMap(); 
  }
  
  inline const glm::mat4& GetLightSpaceMatrix(int i) const { 
    return depth_map_renderers_[i].GetLightSpaceMatrix(); 
  }

  inline GLfloat GetSubfrustaExtent(int i) const {
    return subfrusta_extents_[i];
  }
  
  inline GLfloat GetShadowBias(int i) const { return shadow_biases_[i]; }

  inline void ToggleVisible() {
    visible_ = !visible_;
  }
  
  void Display(); 

 private:
  GLuint map_width_;
  GLuint map_height_;
  Shader shader_; // depth map shader
  Shader debug_shader_; // shader to display depth map textures
  std::vector<GLfloat> subfrusta_extents_;
  std::vector<GLfloat> shadow_biases_;
  std::vector<DepthMapRenderer> depth_map_renderers_;
  bool visible_; // controls if textures are rendered for debugging
  GLuint quadVAO_; // for rendering depth map texture
  GLuint quadVBO_; // for rendering depth map texture

};
} // End namespace TopFun

#endif
