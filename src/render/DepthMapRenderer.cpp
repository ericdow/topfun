#include "utils/GLEnvironment.h"
#include "render/DepthMapRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
DepthMapRenderer::DepthMapRenderer(GLuint map_width, GLuint map_height) : 
  map_width_(map_width), map_height_(map_height) {

  // Create depth texture
  glGenTextures(1, &depth_map_);
  glBindTexture(GL_TEXTURE_2D, depth_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, map_width, 
      map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  // Attach depth texture as FBO's depth buffer
  glGenFramebuffers(1, &depth_mapFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
      depth_map_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

//****************************************************************************80
void DepthMapRenderer::Render(Terrain& terrain, const Sky& sky, 
    Aircraft& aircraft, const Camera& camera, const glm::mat4& proj_view, 
    const Shader& shader) {
  // Render the scene and store the depth buffer
  shader.Use();

  glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(),
        "projection_view"), 1, GL_FALSE, glm::value_ptr(proj_view));

  // Get the size of the viewport
  glm::ivec4 viewport = GLEnvironment::GetViewport();
  GLint screen_width = viewport[2];
  GLint screen_height = viewport[3];

  glViewport(0, 0, map_width_, map_height_);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glClear(GL_DEPTH_BUFFER_BIT);
  DrawScene(terrain, sky, aircraft, camera, nullptr, &shader); 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Reset viewport
  glViewport(0, 0, screen_width, screen_height);
}

} // End namespace TopFun

