#ifndef CLOUDRENDERER_H
#define CLOUDRENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "render/Camera.h"
#include "render/DepthMapRenderer.h"
#include "sky/NoiseCube.h"

namespace TopFun {

class Terrain;
class Sky;
class Aircraft;

class CloudRenderer {
 public:
  CloudRenderer(GLuint map_width, GLuint map_height);

  ~CloudRenderer();

  void Render(Terrain& terrain, const Sky& sky, Aircraft& aircraft, 
      const Camera& camera);
  
  //**************************************************************************80
  //! \brief GetCloudStartEnd - gets the start and end altitudes of the clouds
  //**************************************************************************80
  inline const std::array<float,2>& GetCloudStartEnd() const {
    return cloud_start_end_; 
  }
  
  //**************************************************************************80
  //! \brief SetCloudStartEnd - sets the start and end altitudes of the clouds
  //**************************************************************************80
  inline void SetCloudStartEnd(const std::array<float,2>& cloud_start_end) {
    cloud_start_end_ = cloud_start_end;
  }

 private:
  GLuint map_width_;
  GLuint map_height_;
  Shader depth_map_shader_; // for generating depth map
  Shader shader_; // for rendering the clouds
  DepthMapRenderer depth_map_renderer_;
  GLuint quadVAO_; // for rendering cloud texture
  GLuint quadVBO_; // for rendering cloud texture
  std::array<float,2> cloud_start_end_; // start and end altitudes of clouds
  float l_stop_max_;

  // Cloud texture data
  NoiseCube detail_;
  float detail_scale_; // world space dimensions of the detail texture

  void SetShaderData(const Sky& sky, const Camera& camera) const;

};
} // End namespace TopFun

#endif
