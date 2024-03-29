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

  void RenderToTexture(Terrain& terrain, const Sky& sky, Aircraft& aircraft, 
      const Camera& camera);

  void BlendWithScene() const;
  
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
  Shader raymarch_shader_; // for rendering the clouds
  Shader blend_shader_; // for blending the clouds with the scene
  DepthMapRenderer depth_map_renderer_;
  GLuint quadVAO_; // for rendering cloud texture
  GLuint quadVBO_; // for rendering cloud texture
  GLuint texture_curr_; // current cloud texture to be drawn to screen
  GLuint texture_prev_; // previous cloud texture for temporal anti-aliasing
  GLuint depth_curr_; // current cloud depth for temporal anti-aliasing
  GLuint depth_prev_; // previous cloud depth for temporal anti-aliasing
  GLuint cloudFBO_;

  // Cloud parameters
  std::array<float,2> cloud_start_end_; // start and end altitudes of clouds
  float l_stop_max_;
  float max_cloud_height_; // tallest possible cloud height
  glm::mat4 proj_view_prev_; // needed for temporal anti-aliasing

  // Cloud texture data
  GLuint weather_; // 2D texture with coverage/height/altitude
  float weather_scale_; // world space dimensions of the weather texture
  NoiseCube shape_;
  float shape_scale_; // world space dimensions of the shape texture
  NoiseCube detail_;
  float detail_scale_; // world space dimensions of the detail texture

  //**************************************************************************80
  //! \brief SetShaderData - send the data to be rendered to the shader
  //**************************************************************************80
  void SetShaderData(const Sky& sky, const Camera& camera);
  
  //**************************************************************************80
  //! \brief GenerateWeatherTexture - generate the data for the weather texture
  //! \param[in] size - number of pixels in x/y dimensions of the texture
  //**************************************************************************80
  void GenerateWeatherTexture(unsigned size);

};
} // End namespace TopFun

#endif
