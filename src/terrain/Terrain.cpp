#include <SOIL.h>
#include <glm/gtc/type_ptr.hpp>

#include "terrain/Terrain.h"
#include "sky/Sky.h"
#include "render/ShadowCascadeRenderer.h"

namespace TopFun {
//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
noise::module::Perlin Terrain::perlin_generator_;

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(float l, int ntile, const std::array<float,2>& xz_center0) :
  shader_("shaders/terrain.vs", "shaders/terrain.fs"), ntile_(ntile),
  ltile_(l / ntile), xz_center0_(xz_center0) {
  // Use odd number of tiles to make math easier
  if (ntile_ % 2 == 0) {
    std::string message = "Number of tiles in each direction should be odd\n";
    throw std::invalid_argument(message);
  }

  // Set the noise parameters
  perlin_generator_.SetOctaveCount(5);
  perlin_generator_.SetFrequency(0.04);
  perlin_generator_.SetPersistence(0.5);
  
  // Load the textures
  LoadTextures();
    
  // Set up the tiles
  TerrainTile::SetTileLength(ltile_);
  int half_ntile = (ntile_ - 1) / 2;
  for (int i = -half_ntile; i <= half_ntile; ++i) {
    for (int j = -half_ntile; j <= half_ntile; ++j) {
      tiles_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(ntile_*j + i),
                     std::forward_as_tuple(shader_, 
                       xz_center0_[0] + ltile_*(i - 0.5), 
                       xz_center0_[1] + ltile_*(j - 0.5)));
    }
  }
  tile_bounding_box_ = {{-half_ntile, -half_ntile, half_ntile, half_ntile}};
  UpdateTileConnectivity();
}

//****************************************************************************80
void Terrain::SetXZCenter(const std::array<float,2>& xz_center) {
  bool changed = false;
  // Determine where the new center tile is located
  std::array<int,2> ij_center, ij_center_old;
  ij_center[0] = (int)std::ceil(xz_center[0] - xz_center0_[0]) / ltile_;
  ij_center[1] = (int)std::ceil(xz_center[1] - xz_center0_[1]) / ltile_;
  ij_center_old[0] = (tile_bounding_box_[2] + tile_bounding_box_[0]) / 2;
  ij_center_old[1] = (tile_bounding_box_[3] + tile_bounding_box_[1]) / 2;
  
  // Remove the tiles that are no longer visible
  int half_ntile = (ntile_ - 1) / 2;
  for (int i = tile_bounding_box_[0]; i <= tile_bounding_box_[2]; ++i) {
    for (int j = tile_bounding_box_[1]; j <= tile_bounding_box_[3]; ++j) {
      if (i < ij_center[0] - half_ntile || i > ij_center[0] + half_ntile || 
          j < ij_center[1] - half_ntile || j > ij_center[1] + half_ntile) {
        changed = true;
        tiles_.erase(ntile_*j + i);
      }
    }
  }
  
  // Update the bounding box 
  tile_bounding_box_ = {{ij_center[0] - half_ntile, ij_center[1] - half_ntile, 
    ij_center[0] + half_ntile, ij_center[1] + half_ntile}};

  // Create new tiles and update the connectivity
  for (int i = tile_bounding_box_[0]; i <= tile_bounding_box_[2]; ++i) {
    for (int j = tile_bounding_box_[1]; j <= tile_bounding_box_[3]; ++j) {
      int ix = ntile_*j + i;
      if (tiles_.find(ix) == tiles_.end()) {
        tiles_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(ix),
                       std::forward_as_tuple(shader_, 
                         xz_center0_[0] + ltile_*(i - 0.5), 
                         xz_center0_[1] + ltile_*(j - 0.5)));
      }
    }
  } 
  if (changed) {
    UpdateTileConnectivity();
  }
}

//****************************************************************************80
void Terrain::Draw(Camera const& camera, const Sky& sky, 
    const ShadowCascadeRenderer* pshadow_renderer, const Shader* shader) {
  if (!shader) {
    // Send data to the shaders
    SetShaderData(camera, sky, *pshadow_renderer);
  }
  else {
    shader->Use();
    glm::mat4 model = GetModelMatrix(camera);
    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "model"), 1, 
        GL_FALSE, glm::value_ptr(model));
  }
  
  // Loop over tiles and update LoD
  for (auto& t : tiles_) {
    t.second.UpdateLoD(camera.GetPosition());
  }

  // Loop over tiles and draw
  for (auto& t : tiles_) {
    t.second.Draw();
  }
}

//****************************************************************************80
float Terrain::GetHeight(float x, float z) {
  return 10.0f * perlin_generator_.GetValue(x/10, z/10, 0.5);
}

//****************************************************************************80
float Terrain::GetBoundingHeight(float x, float z) const {
  // Determine which tile we're in
  int i = (int)((x - xz_center0_[0] + 0.5*ltile_) / ltile_);
  int j = (int)((z - xz_center0_[1] + 0.5*ltile_) / ltile_);
  int ix = ntile_*j + i;
  return tiles_.at(ix).GetBoundingHeight();
}

//****************************************************************************80
glm::vec3 Terrain::GetNormal(float x, float z) {
  float eps = 1.0e-5f;
  return glm::normalize(glm::vec3(
        (Terrain::GetHeight(x+eps,z) - Terrain::GetHeight(x,z))/eps,
        1.0f,
        (Terrain::GetHeight(x,z+eps) - Terrain::GetHeight(x,z))/eps));
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Terrain::LoadTextures() {
  // TODO give better path...
  std::vector<std::string> filepaths = {
    "../../../assets/textures/seamless_grass2.jpg",
    "../../../assets/textures/seamless_grass.jpg",
    "../../../assets/textures/seamless_grassydirt.jpg"};
  textures_.resize(filepaths.size());
  for (GLuint i = 0; i < filepaths.size(); ++i) {
    // Load and create a texture 
    glGenTextures(1, textures_.data() + i);
    glBindTexture(GL_TEXTURE_2D, textures_[i]); 
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
        GL_LINEAR_MIPMAP_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image(filepaths[i].c_str(), &width, 
        &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
  }
  // Clean up
  glBindTexture(GL_TEXTURE_2D, 0);
}

//****************************************************************************80
void Terrain::SetShaderData(Camera const& camera, const Sky& sky, 
    const ShadowCascadeRenderer& shadow_renderer) {
  // Activate shader
  shader_.Use();
  // Set model/view/projection uniforms  
  glm::mat4 model = GetModelMatrix(camera);
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "model"), 1, 
      GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"),
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

  // Set material uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), 
        "material.specular"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), 
        "material.shiny"), 1.0f);

  // Set lighting uniforms
  const glm::vec3& sun_dir = sky.GetSunDirection();
  const glm::vec3& sun_color = sky.GetSunColor();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.direction"),
      sun_dir.x, sun_dir.y, sun_dir.z);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.ambient"), 
      0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.diffuse"), 
      0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.specular"), 
      0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);

  // Set fog uniforms
  const glm::vec3& fog_color = sky.GetFogColor();
  const std::array<float,2>& fog_start_end = sky.GetFogStartEnd();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "fog.Color"),
      fog_color.x, fog_color.y, fog_color.z);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.Start"), 
      fog_start_end[0]);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.End"), 
      fog_start_end[1]);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "fog.Equation"), 
      sky.GetFogEquation());
  
  // Set the camera position uniform
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "viewPos"), 
      0.0f, 0.0f, 0.0f);
    
  // Bind the texture data
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[0]);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "grassTexture0"), 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures_[1]);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "grassTexture1"), 1);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, textures_[2]);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "grassTexture2"), 2);
  
  // Set the shadow data
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "num_cascades"), 
      shadow_renderer.GetNumCascades());
  for (int i = 0; i < shadow_renderer.GetNumCascades(); ++i) { 
    // Send the depth maps
    glActiveTexture(GL_TEXTURE3 + i);
    glBindTexture(GL_TEXTURE_2D, shadow_renderer.GetDepthMap(i));
    std::string tmp = "depthMap[" + std::to_string(i) + "]";
    glUniform1i(glGetUniformLocation(shader_.GetProgram(), tmp.c_str()), 3 + i);
    // Send the subfrusta end points
    tmp = "subfrusta_extents[" + std::to_string(i) + "]";
    glUniform1f(glGetUniformLocation(shader_.GetProgram(), tmp.c_str()), 
        shadow_renderer.GetSubfrustaExtent(i));
    // Send the light space matrices
    tmp = "lightSpaceMatrix[" + std::to_string(i) + "]";
    glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), tmp.c_str()), 
        1, GL_FALSE, glm::value_ptr(shadow_renderer.GetLightSpaceMatrix(i)));
    // Send the shadow biases
    tmp = "shadow_bias[" + std::to_string(i) + "]";
    glUniform1f(glGetUniformLocation(shader_.GetProgram(), tmp.c_str()), 
        shadow_renderer.GetShadowBias(i));
  }
  glm::vec3 camera_front = camera.GetFront();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "cameraFront"), 
      camera_front.x, camera_front.y, camera_front.z);
  glm::vec3 frustum_origin = camera.GetFrustumOrigin();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "frustumOrigin"), 
      frustum_origin.x, frustum_origin.y, frustum_origin.z);
  glm::vec3 frustum_terminus = camera.GetFrustumTerminus();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "frustumTerminus"), 
      frustum_terminus.x, frustum_terminus.y, frustum_terminus.z);
}

//****************************************************************************80
void Terrain::UpdateTileConnectivity() {
  for (int i = tile_bounding_box_[0]; i <= tile_bounding_box_[2]; ++i) {
    for (int j = tile_bounding_box_[1]; j <= tile_bounding_box_[3]; ++j) {
      int ix = ntile_*j + i;
      if (i > tile_bounding_box_[0]) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile_*j + i - 1), 3);
      }
      if (i < tile_bounding_box_[2]) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile_*j + i + 1), 1);
      }
      if (j > tile_bounding_box_[1]) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile_*(j-1) + i), 2);
      }
      if (j < tile_bounding_box_[3]) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile_*(j+1) + i), 0);
      }
    }
  }
}

} // End namespace TopFun
