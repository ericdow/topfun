#ifndef NOISECUBE_H
#define NOISECUBE_H

#include <array>
#include <vector>

#include <GL/glew.h>

// Class for generating tileable noise textures for cloud generation

namespace TopFun {

class NoiseCube {
 
 public:
  // Helper struct for storing noise parameters
  struct NoiseParams {
    NoiseParams(unsigned n_octaves = 8, float frequency = 1.0f, 
        float persistence = 1.0f) : 
      n_octaves_(n_octaves), 
      frequency_(frequency), 
      persistence_(persistence) {}
    unsigned n_octaves_;
    float frequency_;
    float persistence_;
  };
   
  //**************************************************************************80
  //! \brief NoiseCube - Constructor
  //! \param[in] size - x,y,z dimensions of the cube
  //! \param[in] type - type of noise for each component ("perlin" or "worley")
  //! \param[in] params - vector of noise parameters for each component
  //**************************************************************************80
  NoiseCube(const std::array<unsigned,3>& size, 
      const std::vector<std::string>& type, 
      const std::vector<std::vector<NoiseParams>>& params);
  
  //**************************************************************************80
  //! \brief ~NoiseCube - Destructor
  //**************************************************************************80
  ~NoiseCube() = default;
  
  //**************************************************************************80
  //! \brief GetTexture - Returns the 3D texture 
  //**************************************************************************80
  GLuint GetTexture() const { return texture_; }
  
 private:
  GLuint texture_;
  
  //**************************************************************************80
  //! \brief GenerateWorleyNoise - generate tileable Worley noise data
  //! \param[in] params - noise parameters for each dimension
  //**************************************************************************80
  std::vector<float> GenerateWorleyNoise(const std::array<unsigned,3>& size,
      const std::vector<NoiseParams>& params) const;
  
  //**************************************************************************80
  //! \brief GeneratePerlinNoise - generate tileable Perlin noise data
  //! \param[in] params - noise parameters for each dimension
  //**************************************************************************80
  std::vector<float> GeneratePerlinNoise(const std::array<unsigned,3>& size,
      const std::vector<NoiseParams>& params) const;

};
} // End namespace TopFun

#endif
