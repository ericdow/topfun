#include <algorithm>

#include "render/ShadowCascadeRenderer.h"
#include "render/SceneRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
ShadowCascadeRenderer::ShadowCascadeRenderer(GLuint map_width, 
    GLuint map_height, const std::vector<GLfloat>& subfrusta_extents) :
  map_width_(map_width), map_height_(map_height), 
  shader_("shaders/depthmap.vs", "shaders/depthmap.fs"),
  debug_shader_("shaders/debug_quad.vs", "shaders/debug_quad.fs"),
  subfrusta_extents_(subfrusta_extents), visible_(true) {

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

  // Construct the depth map renderers
  depth_map_renderers_.emplace_back(0.0, subfrusta_extents_[0], map_width, 
      map_height);
  for (std::size_t i = 1; i < subfrusta_extents_.size(); ++i) {
    depth_map_renderers_.emplace_back(subfrusta_extents_[i-1], 
        subfrusta_extents_[i], map_width, map_height);
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
  for (auto& d : depth_map_renderers_) {
    d.Render(terrain, sky, aircraft, camera, light_dir, shader_);
  }
}

//****************************************************************************80
void ShadowCascadeRenderer::Display() {
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

} // End namespace TopFun

