#include <SOIL.h>
#include <glm/gtc/type_ptr.hpp>

#include "terrain/Terrain.h"
#include "terrain/Sky.h"
#include "render/DepthMapRenderer.h"

namespace TopFun {
//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
noise::module::Perlin Terrain::perlin_generator_;

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(GLfloat l, GLuint ntile) : 
  shader_("shaders/terrain.vs", "shaders/terrain.fs") {
  // Set the noise parameters
  perlin_generator_.SetOctaveCount(5);
  perlin_generator_.SetFrequency(0.04);
  perlin_generator_.SetPersistence(0.5);

  // Load the textures
  LoadTextures();
    
  // Set up the tiles
  GLfloat l_tile = l / ntile;
  TerrainTile::SetTileLength(l_tile);
  for (GLuint i = 0; i < ntile; ++i) {
    for (GLuint j = 0; j < ntile; ++j) {
      tiles_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(ntile*j + i),
                     std::forward_as_tuple(shader_, l_tile*i, l_tile*j));
    }
  }
  for (GLuint i = 0; i < ntile; ++i) {
    for (GLuint j = 0; j < ntile; ++j) {
      GLuint ix = ntile*j + i;
      if (i > 0) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile*j + i - 1), 3);
      }
      if (i < ntile - 1) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile*j + i + 1), 1);
      }
      if (j > 0) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile*(j-1) + i), 2);
      }
      if (j < ntile - 1) {
        tiles_.at(ix).SetNeighborPointer(&tiles_.at(ntile*(j+1) + i), 0);
      }
    }
  }
}

//****************************************************************************80
void Terrain::Draw(Camera const& camera, const Sky& sky, 
    const DepthMapRenderer& depthmap_renderer, const Shader* shader) {
  if (!shader) {
    // Send data to the shaders
    SetShaderData(camera, sky, depthmap_renderer);
  }
  else {
    shader->Use();
    glm::mat4 model;
    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "model"), 1, 
        GL_FALSE, glm::value_ptr(model));
  }
  
  // Loop over tiles and update LoD
  for (auto& t : tiles_) {
    t.second.UpdateLoD(camera.GetPosition());
  }

  // Loop over tiles and draw
  for (auto& t : tiles_) {
    t.second.Draw(camera);
  }
}

//****************************************************************************80
GLfloat Terrain::GetHeight(GLfloat x, GLfloat z) {
  return 4.0f * perlin_generator_.GetValue(x/10, z/10, 0.5);
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
    const DepthMapRenderer& depthmap_renderer) {
  // Activate shader
  shader_.Use();
  // Set view/projection uniforms  
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
  glm::vec3 camera_pos = camera.GetPosition();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "viewPos"), 
      camera_pos.x, camera_pos.y, camera_pos.z);
    
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
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), 
        "lightSpaceMatrix"), 1, GL_FALSE, 
      glm::value_ptr(depthmap_renderer.GetLightSpaceMatrix()));
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, depthmap_renderer.GetDepthMap());
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "depthMap"), 3);
}

} // End namespace TopFun
