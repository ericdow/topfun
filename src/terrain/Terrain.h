#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "utils/Camera.h"

namespace TopFun {

class Terrain {
 
 public:
  //**************************************************************************80
  //! \brief Terrain - Constructor for empy terrain object
  //**************************************************************************80
  Terrain();
  
  //**************************************************************************80
  //! \brief ~Terrain - Destructor
  //**************************************************************************80
  ~Terrain() = default;
  
  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a bunch of (x,y) locations
  //**************************************************************************80
  std::vector<float> GetHeight(std::vector<std::array<float,2>> const& xy) 
    const;

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(Camera const& camera);

 private:
  Shader shader_;
  GLuint texture_;
  
  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera);
  
  //**************************************************************************80
  //! \brief LoadTexture - loads the texture using SOIL
  //**************************************************************************80
  void LoadTexture();

};
} // End namespace TopFun

#endif
