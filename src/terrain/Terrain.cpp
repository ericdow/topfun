#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
noise::module::Perlin Terrain::perlin_generator_;

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(GLfloat lx, GLfloat lz) : lx_(lx), lz_(lz),
  shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  // Set the noise parameters
  perlin_generator_.SetOctaveCount(7);
  perlin_generator_.SetFrequency(0.3);
  perlin_generator_.SetPersistence(0.5);
    
  // Set up the tiles
  GLfloat l_tile = 100.0;
  TerrainTile::SetTileLength(l_tile);
  // TODO
  tiles_.emplace(std::piecewise_construct,
                 std::forward_as_tuple(0),
                 std::forward_as_tuple(shader_, 0.0, 0.0));
  tiles_.emplace(std::piecewise_construct,
                 std::forward_as_tuple(1),
                 std::forward_as_tuple(shader_, l_tile, 0.0));
  tiles_.emplace(std::piecewise_construct,
                 std::forward_as_tuple(2),
                 std::forward_as_tuple(shader_, 0, l_tile));
  tiles_.emplace(std::piecewise_construct,
                 std::forward_as_tuple(3),
                 std::forward_as_tuple(shader_, l_tile, l_tile));
  tiles_.at(0).SetNeighborPointer(&tiles_.at(1), 1);
  tiles_.at(1).SetNeighborPointer(&tiles_.at(0), 3);
  tiles_.at(0).SetNeighborPointer(&tiles_.at(2), 0);
  tiles_.at(2).SetNeighborPointer(&tiles_.at(0), 2);
  tiles_.at(2).SetNeighborPointer(&tiles_.at(3), 1);
  tiles_.at(3).SetNeighborPointer(&tiles_.at(2), 3);
  tiles_.at(1).SetNeighborPointer(&tiles_.at(3), 0);
  tiles_.at(3).SetNeighborPointer(&tiles_.at(1), 2);
  tiles_.at(0).SetLoD(0);
  tiles_.at(1).SetLoD(1);
  tiles_.at(2).SetLoD(2);
  tiles_.at(3).SetLoD(3);

}

//****************************************************************************80
Terrain::~Terrain() {}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  // Activate shader
  shader_.Use();

  // Send data to the shaders
  SetShaderData(camera);

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
void Terrain::SetShaderData(Camera const& camera) {
  // Set view/projection uniforms  
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"),
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

  // Set material uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), 
        "material.color"), 0.15f, 0.5f, 0.25f);
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
