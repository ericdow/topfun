#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <array>
#include <unordered_map>

#include <glm/glm.hpp>

#include "module/perlin.h"

#include "shaders/Shader.h"
#include "render/Camera.h"
#include "terrain/TerrainTile.h"

namespace TopFun {

class ShadowCascadeRenderer;
class Sky;

class Terrain {
 
 public:
  //**************************************************************************80
  //! \brief Terrain - Constructor for empty terrain object
  //! \param[in] l - length of terrain in the x/z directions
  //! \param[in] ntile - number of terrain tiles in the x/z directions
  //! \param[in] xz_center0 - starting location of center of rendered terrain
  //**************************************************************************80
  Terrain(GLfloat l, int ntile, const std::array<float,2>& xz_center0);
  
  //**************************************************************************80
  //! \brief ~Terrain - Destructor
  //**************************************************************************80
  ~Terrain() = default;
  
  //**************************************************************************80
  //! \brief SetXZCenter - Update the location of the center of rendered terrain
  //! \param[in] xz_center - new location of center of rendered terrain
  //**************************************************************************80
  void SetXZCenter(const std::array<float,2>& xz_center); 

  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a some (x,y) location
  //**************************************************************************80
  static GLfloat GetHeight(GLfloat x, GLfloat z);

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(const Camera& camera, const Sky& sky, 
      const ShadowCascadeRenderer* pshadow_renderer, const Shader* shader=NULL);

 private:
  Shader shader_;
  int ntile_;
  GLfloat ltile_;
  std::array<float,2> xz_center0_; // center of terrain
  std::array<int,4> tile_bounding_box_; // bounding box in tile coordinates
  static noise::module::Perlin perlin_generator_;
  std::unordered_map<int,TerrainTile> tiles_;
  std::vector<GLuint> textures_;
  
  //**************************************************************************80
  //! \brief LoadTextures - load the terrain textures
  //**************************************************************************80
  void LoadTextures();
  
  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera, const Sky& sky,
      const ShadowCascadeRenderer& shadow_renderer);
  
  //**************************************************************************80
  //! \brief GetModelMatrix - get the model matrix for the terrain
  //! \param[in] camera - reference to the camera
  //**************************************************************************80
  inline glm::mat4 GetModelMatrix(const Camera& camera) const {
    return glm::translate(glm::mat4(), (glm::vec3)-camera.GetPosition());
  }

  //**************************************************************************80
  //! \brief UpdateTileConnectivity - update tile neighbor pointers
  //**************************************************************************80
  void UpdateTileConnectivity();

};
} // End namespace TopFun

#endif
