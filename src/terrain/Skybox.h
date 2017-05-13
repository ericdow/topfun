#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "utils/Camera.h"

namespace TopFun {

class Skybox {
 
 public:
  //**************************************************************************80
  //! \brief Skybox - Constructor
  //**************************************************************************80
  Skybox();
  
  //**************************************************************************80
  //! \brief ~Skybox - Destructor
  //**************************************************************************80
  ~Skybox();

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(Camera const& camera);

 private:
  Shader shader_;
  GLuint cubemap_texture_;
  GLuint VAO_;
  
  //**************************************************************************80
  //! \brief SetShaderData - sends the uniforms required by the shader
  //**************************************************************************80
  void SetShaderData(Camera const& camera);
  
  //**************************************************************************80
  //! \brief LoadCubemap - loads the texture data for the cubemap
  //**************************************************************************80
  void LoadCubemap(std::vector<const GLchar*> const& faces);

};
} // End namespace TopFun

#endif
