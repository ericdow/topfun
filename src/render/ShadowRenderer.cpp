#include "utils/ShadowRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
ShadowRenderer::ShadowRenderer(GLuint shadow_width, GLuint shadow_height) :
  shadow_width_(shadow_width), shadow_height_(shadow_height), 
  shader_("shaders/depthmap.vs", "shaders/depthmap.fs") {
  // Create depth texture
  glGenFramebuffers(1, &depth_mapFBO_);
  glGenTextures(1, &depth_map_);
  glBindTexture(GL_TEXTURE_2D, depth_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width_, 
      shadow_height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
}

//****************************************************************************80
void ShadowRenderer::RenderDepthMap(const glm::vec3& light_pos) {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render scene from light's point of view
  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  // TODO these control how far the shadows travel
  float near_plane = 1.0f, far_plane = 2000.0f;
  lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, 
      far_plane);
  lightView = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
  lightSpaceMatrix = lightProjection * lightView;
  shader_.setMat4("lightSpaceMatrix", lightSpaceMatrix);

  glViewport(0, 0, shadow_width_, shadow_height_);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glClear(GL_DEPTH_BUFFER_BIT);
  renderScene(simpleDepthShader);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Reset viewport
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // End namespace TopFun

