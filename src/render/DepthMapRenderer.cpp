#include "render/DepthMapRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
DepthMapRenderer::DepthMapRenderer(GLuint map_width, GLuint map_height) :
  map_width_(map_width), map_height_(map_height), 
  shader_("shaders/depthmap.vs", "shaders/depthmap.fs") {
  // Create depth texture
  glGenFramebuffers(1, &depth_mapFBO_);
  glGenTextures(1, &depth_map_);
  glBindTexture(GL_TEXTURE_2D, depth_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, map_width_, 
      map_height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // Attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
      depth_map_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

//****************************************************************************80
void DepthMapRenderer::Render(Terrain& terrain, Sky& sky, 
    Aircraft& aircraft, const Camera& camera, const glm::vec3& light_pos) {
  shader_.Use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render scene from light's point of view
  glm::mat4 light_projection, light_view;
  // TODO these control how far the shadows travel
  float near_plane = 1.0f, far_plane = 100.0f;
  light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, 
      far_plane);
  light_view = glm::lookAt(light_pos, glm::vec3(0.0f), 
      glm::vec3(0.0f, 1.0f, 0.0f));
  light_space_matrix_ = light_projection * light_view;
 
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(),
        "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix_));

  // Get the size of the viewport
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLint screen_width = viewport[2];
  GLint screen_height = viewport[3];

  glViewport(0, 0, map_width_, map_height_);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glClear(GL_DEPTH_BUFFER_BIT);
  DrawScene(terrain, sky, aircraft, camera, *this, &shader_); 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Reset viewport
  glViewport(0, 0, screen_width, screen_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Grab and print the depth texture data
  std::vector<GLfloat> texture_data(map_width_ * map_height_);
  glBindTexture(GL_TEXTURE_2D, depth_map_);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 
      texture_data.data());
  static bool print = true;
  if (print) {
    for (auto const& d : texture_data) {
      std::cout << d << std::endl;
    }
    print = false;
  }
}

} // End namespace TopFun

