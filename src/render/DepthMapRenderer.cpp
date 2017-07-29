#include "render/DepthMapRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
DepthMapRenderer::DepthMapRenderer(GLuint map_width, GLuint map_height) :
  map_width_(map_width), map_height_(map_height), 
  shader_("shaders/depthmap.vs", "shaders/depthmap.fs"),
  debug_shader_("shaders/debug_quad.vs", "shaders/debug_quad.fs"),
  visible_(true) {
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

  // Set up the quad for rendering the depth map for debugging
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
void DepthMapRenderer::Render(Terrain& terrain, Sky& sky, 
    Aircraft& aircraft, const Camera& camera, const glm::vec3& light_dir) {
  // Render scene from light's point of view
  shader_.Use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Determine light-space bounding box of the camera frustrum
  glm::mat4 light_space = glm::lookAt(light_dir, glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 inv_light_space = glm::inverse(light_space);
  std::array<glm::vec3,8> frustrum_vertices = camera.GetFrustrumVertices();
  glm::vec3 vls_min(std::numeric_limits<float>::max());
  glm::vec3 vls_max(std::numeric_limits<float>::lowest());
  for (const auto& v : frustrum_vertices) {
    glm::vec4 tmp = inv_light_space * glm::vec4(v, 1.0f);
    glm::vec3 vls = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
    for (int d = 0; d < 3; ++d) {
      if (vls[d] < vls_min[d])
        vls_min[d] = vls[d];
      if (vls[d] > vls_max[d])
        vls_max[d] = vls[d];
    }
  }

  // Determine the world-space center of the bounding box
  // glm::vec3 vls_mid = 0.5f * (vls_max + vls_min);
  // glm::vec4 tmp = light_space * glm::vec4(vls_mid, 1.0f);
  // vls_mid = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
  
  glm::vec3 vls_mid(0.0f);
  for (const auto& v : frustrum_vertices) {
    vls_mid += v / 8.0f;
  }
  vls_mid = camera.GetPosition()+scale_factor_*(vls_mid - camera.GetPosition());
  
  // glm::vec3 vls_mid = camera.GetPosition();
  
  // for (int d = 0; d < 3; ++d) {
  //   std::cout << vls_max[d] << " ";
  // }
  // std::cout << std::endl;
  // 
  // for (int d = 0; d < 3; ++d) {
  //   std::cout << vls_min[d] << " ";
  // }
  // std::cout << std::endl;
  // 
  // for (int d = 0; d < 3; ++d) {
  //   std::cout << vls_mid[d] << " ";
  // }
  // std::cout << std::endl;
  // std::cout << std::endl;
 
  // Construct an orthographic projection matrix using the bounding box
  GLfloat width  = scale_factor_ * (vls_max.x - vls_min.x); 
  GLfloat height = scale_factor_ * (vls_max.y - vls_min.y);
  GLfloat depth  = scale_factor_ * (vls_max.z - vls_min.z);
  // TODO
  // width = 50.0f;
  // height = 50.0f;
  // depth = 50.0f;
  // width /= scale_factor_; height /= scale_factor_; depth /= scale_factor_;
  glm::mat4 light_projection = glm::ortho(-width / 2.0f, width / 2.0f, 
      -height / 2.0f, height / 2.0f, -depth / 2.0f, depth / 2.0f);

  glm::mat4 light_view = glm::lookAt(vls_mid, 
      vls_mid - light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
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
}

//****************************************************************************80
void DepthMapRenderer::Display() {
  if (visible_) {
    // Grab the original viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Set the viewport to only show the upper right corner 
    GLuint x0 = 3*viewport[2]/4;
    GLuint y0 = 3*viewport[3]/4;
    glViewport(x0,y0,viewport[2]/4,viewport[3]/4);
    glScissor(x0,y0,viewport[2]/4,viewport[3]/4);
    glEnable(GL_SCISSOR_TEST);

    // Render the depth map texture
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	  debug_shader_.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_map_);
    glUniform1i(glGetUniformLocation(debug_shader_.GetProgram(), "depthMap"), 
        0);
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    // Reset viewport
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, viewport[2], viewport[3]);
  }
}

} // End namespace TopFun

