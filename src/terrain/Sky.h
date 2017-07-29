#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "render/Camera.h"

namespace TopFun {

class Sky {
 
 public:
  //**************************************************************************80
  //! \brief Sky - Constructor
  //**************************************************************************80
  Sky();
  
  //**************************************************************************80
  //! \brief ~Sky - Destructor
  //**************************************************************************80
  ~Sky();

  //**************************************************************************80
  //! \brief Draw - draws the terrain
  //**************************************************************************80
  void Draw(Camera const& camera);
  
  //**************************************************************************80
  //! \brief GetSunDirection - gets the direction of the sun
  //**************************************************************************80
  inline const glm::vec3& GetSunDirection() const { return sun_dir_; }
  
  //**************************************************************************80
  //! \brief SetSunDirection - sets the direction of the sun
  //**************************************************************************80
  inline void SetSunDirection(const glm::vec3& sun_dir) { sun_dir_ = sun_dir; }

  //**************************************************************************80
  //! \brief GetSunColor - gets the color of the sun
  //**************************************************************************80
  inline const glm::vec3& GetSunColor() const { return sun_color_; }
  
  //**************************************************************************80
  //! \brief SetSunColor - sets the color of the sun
  //**************************************************************************80
  inline void SetSunColor(const glm::vec3& sun_color) {sun_color_ = sun_color;}
  
  //**************************************************************************80
  //! \brief GetFogColor - gets the color of the fog
  //**************************************************************************80
  inline const glm::vec3& GetFogColor() const { return fog_color_; }
  
  //**************************************************************************80
  //! \brief SetFogColor - sets the color of the fog
  //**************************************************************************80
  inline void SetFogColor(const glm::vec3& fog_color) {fog_color_ = fog_color;}
  
  //**************************************************************************80
  //! \brief GetFogStartEnd - gets the start and end distances of the fog
  //**************************************************************************80
  inline const std::array<float,2>& GetFogStartEnd() const {
    return fog_start_end_; 
  }
  
  //**************************************************************************80
  //! \brief SetFogStartEnd - sets the start and end distances of the fog
  //**************************************************************************80
  inline void SetFogStartEnd(const std::array<float,2>& fog_start_end) {
    fog_start_end_ = fog_start_end;
  }
  
  //**************************************************************************80
  //! \brief GetFogEquation - gets the equation index used to compute fog
  //**************************************************************************80
  inline GLuint GetFogEquation() const { return fog_eq_; }
  
  //**************************************************************************80
  //! \brief SetFogEquation - sets the equation index used to compute fog
  //**************************************************************************80
  inline void SetFogEquation(GLuint fog_eq) { fog_eq_ = fog_eq; }

 private:
  Shader shader_;
  GLuint cubemap_texture_;
  GLuint VAO_;
  glm::vec3 sun_dir_; // vector from sun to world
  glm::vec3 sun_color_; // color of sunlight
  glm::vec3 fog_color_; // color of fog
  std::array<float,2> fog_start_end_; // start and end distances of fog
  GLuint fog_eq_; // equation index used to compute fog

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
