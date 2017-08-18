#include "render/DepthMapRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
DepthMapRenderer::DepthMapRenderer(GLfloat subfrustum_near, 
    GLfloat subfrustum_far, GLuint map_width, GLuint map_height) : 
  subfrustum_near_(subfrustum_near), subfrustum_far_(subfrustum_far),
  map_width_(map_width), map_height_(map_height) {
  
  // Check that the subfrusta is valid
  if (subfrustum_near_ > subfrustum_far_ 
      || subfrustum_near_ < 0.0f || subfrustum_near_ > 1.0f 
      || subfrustum_far_ < 0.0f || subfrustum_far_ > 1.0f) {
    std::string message = "Invalid subfrusta\n";
    throw std::invalid_argument(message);
  }

  // Create depth texture
  glGenFramebuffers(1, &depth_mapFBO_);
  glGenTextures(1, &depth_map_);
  glBindTexture(GL_TEXTURE_2D, depth_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, map_width, 
      map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
    Aircraft& aircraft, const Camera& camera, const glm::vec3& light_dir, 
    const Shader& shader) {
  // Render scene from light's point of view
  shader.Use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Determine light-space bounding box of the camera frustum
  glm::mat4 light_space = glm::lookAt(light_dir, glm::vec3(0.0f, 0.0f, 0.0f), 
      glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 inv_light_space = glm::inverse(light_space);
  std::array<glm::vec3,8> frustum_vertices = camera.GetFrustumVertices();
  // Set near plane
  for (int i = 0; i < 4; ++i) {
    frustum_vertices[i] = (1.0f - subfrustum_near_) * frustum_vertices[i] + 
      subfrustum_near_ * frustum_vertices[i+4];
  }
  for (int i = 4; i < 8; ++i) {
    frustum_vertices[i] = subfrustum_far_ * frustum_vertices[i] + 
      (1.0f - subfrustum_far_) * frustum_vertices[i-4];
  }
  glm::vec3 v_min(std::numeric_limits<float>::max());
  glm::vec3 v_max(std::numeric_limits<float>::lowest());
  for (const auto& v : frustum_vertices) {
    for (int d = 0; d < 3; ++d) {
      if (v[d] < v_min[d])
        v_min[d] = v[d];
      if (v[d] > v_max[d])
        v_max[d] = v[d];
    }
  }
  std::array<glm::vec3,8> bb_vertices = {
    glm::vec3(v_min.x, v_min.y, v_min.z), 
    glm::vec3(v_max.x, v_min.y, v_min.z), 
    glm::vec3(v_max.x, v_max.y, v_min.z), 
    glm::vec3(v_min.x, v_max.y, v_min.z), 
    glm::vec3(v_min.x, v_min.y, v_max.z), 
    glm::vec3(v_max.x, v_min.y, v_max.z), 
    glm::vec3(v_max.x, v_max.y, v_max.z), 
    glm::vec3(v_min.x, v_max.y, v_max.z)}; 
  glm::vec3 vls_min(std::numeric_limits<float>::max());
  glm::vec3 vls_max(std::numeric_limits<float>::lowest());
  for (const auto& v : bb_vertices) {
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
  glm::vec3 vls_mid = 0.5f * (vls_max + vls_min);
  glm::vec4 tmp = light_space * glm::vec4(vls_mid, 1.0f);
  vls_mid = glm::vec3(tmp.x, tmp.y, tmp.z) / tmp.w;
  
  // Construct an orthographic projection matrix using the bounding box
  GLfloat width  = vls_max.x - vls_min.x; 
  GLfloat height = vls_max.y - vls_min.y;
  GLfloat depth  = vls_max.z - vls_min.z;
  glm::mat4 light_projection = glm::ortho(-width / 2.0f, width / 2.0f, 
      -height / 2.0f, height / 2.0f, -depth / 2.0f, depth / 2.0f);

  glm::mat4 light_view = glm::lookAt(vls_mid, 
      vls_mid - light_dir, glm::vec3(0.0f, 1.0f, 0.0f));
  light_space_matrix_ = light_projection * light_view;
 
  glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(),
        "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(light_space_matrix_));

  // Get the size of the viewport
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLint screen_width = viewport[2];
  GLint screen_height = viewport[3];

  glViewport(0, 0, map_width_, map_height_);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_mapFBO_);
  glClear(GL_DEPTH_BUFFER_BIT);
  DrawScene(terrain, sky, aircraft, camera, nullptr, &shader); 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Reset viewport
  glViewport(0, 0, screen_width, screen_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // End namespace TopFun

