#include "SOIL.h"

#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
noise::module::Perlin Terrain::perlin_generator_;

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(GLfloat l, GLuint ntile) : 
  shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  // Set the noise parameters
  perlin_generator_.SetOctaveCount(5);
  perlin_generator_.SetFrequency(0.2);
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
Terrain::~Terrain() {}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  // Activate shader
  shader_.Use();

  // Send data to the shaders
  SetShaderData(camera);
  
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
  return perlin_generator_.GetValue(x/10, z/10, 0.5);
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
void Terrain::SetShaderData(Camera const& camera) {
  // Set view/projection uniforms  
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"),
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

  // Set material uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), 
        "material.color"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), 
        "material.shininess"), 1.0f);

  // Set lighting uniforms
  // TODO move light direction definition to somewhere higher up
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.direction"),
      -0.3f, -1.0f, 0.0f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.ambient"), 
      0.2f, 0.2f, 0.2f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.diffuse"), 
      0.5f, 0.5f, 0.5f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.specular"), 
      1.0f, 1.0f, 1.0f);

  // Set fog uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "fog.Color"),
      183.0/256.0, 213.0/256.0, 219.0/256.0);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.Start"), 70.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.End"), 200.0f);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "fog.Equation"), 0);
  
  // Set the camera position uniform
  glm::vec3 camera_pos = camera.GetPosition();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "viewPos"), 
      camera_pos.x, camera_pos.y, camera_pos.z);
}

} // End namespace TopFun
