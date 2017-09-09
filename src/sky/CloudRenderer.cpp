#include "sky/CloudRenderer.h"
#include "render/SceneRenderer.h"
#include "utils/GLEnvironment.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
CloudRenderer::CloudRenderer(GLuint map_width, GLuint map_height) :
  map_width_(map_width), map_height_(map_height), 
  depth_map_shader_("shaders/depthmap.vs", "shaders/depthmap.fs"),
  shader_("shaders/clouds.vs", "shaders/clouds.fs"),
  depth_map_renderer_(map_width, map_height),
  cloud_start_end_({100.0f, 150.0f}), l_stop_max_(100.0f),
  detail_({32,32,32}, {"worley","worley","worley"}, {{1,1,1},{2,2,2},{3,3,3}}) {

  // Check that the star and end heights of the clouds are valid
  if (cloud_start_end_[0] > cloud_start_end_[1]) {
    std::string message = "Invalid cloud star/end points\n";
    throw std::invalid_argument(message);
  }

  // Set up the quad for rendering the cloud texture
  float quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  };
  glGenVertexArrays(1, &quadVAO_);
  glGenBuffers(1, &quadVBO_);
  glBindVertexArray(quadVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, 
      GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
      (void*)(3 * sizeof(float)));
}

//****************************************************************************80
void CloudRenderer::Render(Terrain& terrain, const Sky& sky, 
    Aircraft& aircraft, const Camera& camera) {
  // Render the depth map
  glm::mat4 projection_view = camera.GetProjectionMatrix() * 
    camera.GetViewMatrix();
  depth_map_renderer_.Render(terrain, sky, aircraft, camera, projection_view,
      depth_map_shader_);
  // Perform ray-marching and render the clouds
  SetShaderData(sky, camera);
  glBindVertexArray(quadVAO_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
  // TODO
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void CloudRenderer::SetShaderData(const Sky& sky, Camera const& camera) const {
  shader_.Use();
  // Camera related data
  glm::mat4 proj_view = camera.GetProjectionMatrix() * camera.GetViewMatrix();
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projview"),
      1, GL_FALSE, glm::value_ptr(proj_view));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "inv_projview"),
      1, GL_FALSE, glm::value_ptr(glm::inverse(proj_view)));
  glm::ivec4 vp = GLEnvironment::GetViewport();
  glUniform4i(glGetUniformLocation(shader_.GetProgram(), "viewport"), 
      vp[0], vp[1], vp[2], vp[3]);
  std::array<float,2> near_far = camera.GetNearFar();
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "camera_near"), 
      near_far[0]);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "camera_far"), 
      near_far[1]);
  // Depth map of the full scene
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depth_map_renderer_.GetDepthMap());
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "depth_map"), 0);
  // Cloud parameters
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "cloud_start"), 
      cloud_start_end_[0]);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "cloud_end"), 
      cloud_start_end_[1]);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "l_stop_max"), 
      l_stop_max_);
}

} // End namespace TopFun

