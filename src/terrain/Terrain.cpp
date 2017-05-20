#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(GLfloat lx, GLfloat lz) : lx_(lx), lz_(lz),
  shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  // Set up the tiles
  TerrainTile::SetTileLength(20.0);
  // TODO
  tiles_.emplace(std::make_pair(0, shader_));
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
GLfloat Terrain::GetHeight(GLfloat x, GLfloat z) const {
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
        "material.color"), 0.0f, 1.0f, 0.0f);
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
