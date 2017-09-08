#include <algorithm>

#include "utils/GLEnvironment.h"
#include "render/ShadowCascadeRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
ShadowCascadeRenderer::ShadowCascadeRenderer(GLuint map_width, 
    GLuint map_height, const std::vector<GLfloat>& subfrusta_extents,
    const std::vector<GLfloat>& shadow_biases) :
  map_width_(map_width), map_height_(map_height), 
  shader_("shaders/depthmap.vs", "shaders/depthmap.fs"),
  debug_shader_("shaders/debug_quad.vs", "shaders/debug_quad.fs"),
  subfrusta_extents_(subfrusta_extents), shadow_biases_(shadow_biases),
  visible_(true), light_space_matrices_(subfrusta_extents.size()) {

  // Check that the subfrusta are valid
  for (auto ep : subfrusta_extents_) {
    if (ep <= 0.0f || ep > 1.0f) {
      std::string message = "Invalid subfrusta: < 0.0 or > 1.0\n";
      throw std::invalid_argument(message);
    }
  }
  if (!std::is_sorted(subfrusta_extents_.begin(), subfrusta_extents_.end())) {
    std::string message = "Invalid subfrusta: not sorted\n";
    throw std::invalid_argument(message);
  }

  // Check that the input dimensions agree
  if (subfrusta_extents_.size() != shadow_biases_.size()) {
    std::string message = "Inconsistent sizes: subfrusta extents and biases\n";
    throw std::invalid_argument(message);
  }

  // Construct the depth map renderers
  for (std::size_t i = 0; i < subfrusta_extents_.size(); ++i) {
    depth_map_renderers_.emplace_back(map_width, map_height);
  }

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
void ShadowCascadeRenderer::Render(Terrain& terrain, Sky& sky, 
    Aircraft& aircraft, const Camera& camera, const glm::vec3& light_dir) {
  // Update the light-space projection/view matrices
  UpdateLightSpaceMatrices(camera, light_dir);
  // Render the depth maps
  for (std::size_t i = 0; i < depth_map_renderers_.size(); ++i) {
    depth_map_renderers_[i].Render(terrain, sky, aircraft, camera, 
        light_space_matrices_[i], shader_);
  }
}

//****************************************************************************80
void ShadowCascadeRenderer::Display() {
  if (visible_) {
    // Grab the original viewport size
    glm::ivec4 viewport = GLEnvironment::GetViewport();
    
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
    // TODO
    glBindTexture(GL_TEXTURE_2D, depth_map_renderers_[0].GetDepthMap());
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

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void ShadowCascadeRenderer::UpdateLightSpaceMatrices(const Camera& camera, 
    const glm::vec3& light_dir) {
  for (std::size_t f = 0; f < subfrusta_extents_.size(); ++f) {
    // Determine light-space bounding box of the camera frustum
    float subfrustum_near = 0.0f;
    if (f > 0)
      subfrustum_near = subfrusta_extents_[f-1];
    float subfrustum_far = subfrusta_extents_[f];
    glm::mat4 light_space = glm::lookAt(light_dir, glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 inv_light_space = glm::inverse(light_space);
    std::array<glm::vec3,8> frustum_vertices = camera.GetFrustumVertices();
    // Set near plane
    for (int i = 0; i < 4; ++i) {
      frustum_vertices[i] = (1.0f - subfrustum_near) * frustum_vertices[i] + 
        subfrustum_near * frustum_vertices[i+4];
    }
    for (int i = 4; i < 8; ++i) {
      frustum_vertices[i] = subfrustum_far * frustum_vertices[i] + 
        (1.0f - subfrustum_far) * frustum_vertices[i-4];
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
    light_space_matrices_[f] = light_projection * light_view;
  }
}

} // End namespace TopFun

