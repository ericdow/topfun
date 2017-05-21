#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <array>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <noise/module/perlin.h>

#include "shaders/Shader.h"
#include "utils/Camera.h"
#include "terrain/TerrainTile.h"

namespace TopFun {

class Terrain {
 
 public:
  //**************************************************************************80
  //! \brief Terrain - Constructor for empty terrain object
  //! \param[in] lx, lz - length of terrain in the x/z directions
  //**************************************************************************80
  Terrain(GLfloat lx, GLfloat lz);
  
  //**************************************************************************80
  //! \brief ~Terrain - Destructor
  //**************************************************************************80
  ~Terrain();
  
  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a some (x,y) location
  //**************************************************************************80
  static GLfloat GetHeight(GLfloat x, GLfloat z);

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(Camera const& camera);

 private:
  GLfloat lx_, lz_;
  Shader shader_;
  static noise::module::Perlin perlin_generator_;
  std::unordered_map<size_t,TerrainTile> tiles_;
  
  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera);
  
};
} // End namespace TopFun

#endif
