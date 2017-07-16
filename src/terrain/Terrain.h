#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <array>
#include <unordered_map>

#include <glm/glm.hpp>

#include "module/perlin.h"

#include "shaders/Shader.h"
#include "utils/Camera.h"
#include "terrain/TerrainTile.h"

namespace TopFun {

class DepthMapRenderer;
class Sky;

class Terrain {
 
 public:
  //**************************************************************************80
  //! \brief Terrain - Constructor for empty terrain object
  //! \param[in] l - length of terrain in the x/z directions
  //! \param[in] ntile - number of terrain tiles in the x/z directions
  //**************************************************************************80
  Terrain(GLfloat lx, GLuint ntile);
  
  //**************************************************************************80
  //! \brief ~Terrain - Destructor
  //**************************************************************************80
  ~Terrain() = default;
  
  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a some (x,y) location
  //**************************************************************************80
  static GLfloat GetHeight(GLfloat x, GLfloat z);

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(const Camera& camera, const Sky& sky, 
      const DepthMapRenderer& depthmap_renderer, const Shader* shader=NULL);

 private:
  Shader shader_;
  static noise::module::Perlin perlin_generator_;
  std::unordered_map<size_t,TerrainTile> tiles_;
  std::vector<GLuint> textures_;
  
  //**************************************************************************80
  //! \brief LoadTextures - load the terrain textures
  //**************************************************************************80
  void LoadTextures();
  
  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera, const Sky& sky,
      const DepthMapRenderer& depthmap_renderer);
  
};
} // End namespace TopFun

#endif
