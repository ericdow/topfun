#include <cmath>
#include <iostream>

#include <glm/glm.hpp>

#include "sky/NoiseCube.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
NoiseCube::NoiseCube(const std::array<unsigned,3>& size, 
    const std::vector<std::string>& type, 
    const std::vector<std::array<NoiseParams,3>>& params) {
  // Check for consistent data sizes
  unsigned num_components = type.size();
  if ((num_components != 3 && num_components != 4)
      || params.size() != type.size()) {
    std::string message = "Invalid number of texture components\n";
    throw std::invalid_argument(message);
  }

  // Loop through components of the texture and generate noise
  std::vector<unsigned char> pixels(size[0] * size[1] * size[2] * 4, 
      (unsigned char)255);
  for (unsigned c = 0; c < num_components; ++c) {
    // Generate the data for this component
    std::vector<float> data;
    if (type[c].compare("worley") == 0) {
      data = GenerateWorleyNoise(size, params[c]);
    }
    else if (type[c].compare("perlin") == 0) {
      data = GeneratePerlinNoise(size, params[c]);
    }
    else {
      std::string message = "Invalid noise type\n";
      throw std::invalid_argument(message);
    }
    // Convert float data to color data
    for (std::size_t n = 0; n < data.size(); ++n) {
      unsigned char v = (unsigned char)std::round(data[n] * 255);
      pixels[4*n + c] = v;
    }
  }

  // Load the texture
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_3D, texture_); 
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, size[0], size[1], size[2], 0, GL_RGBA, 
      GL_UNSIGNED_BYTE, (GLvoid*)pixels.data());
  glBindTexture(GL_TEXTURE_3D, 0);
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
std::vector<float> NoiseCube::GenerateWorleyNoise(
    const std::array<unsigned,3>& size,
    const std::array<NoiseParams,3>& params) const {
  // Check for data size consistency
  std::array<unsigned,3> n_cells;
  glm::vec3 pixel_size;
  float cell_size = std::sqrt(2.0) / 2.0;
  for (int d = 0; d < 3; ++d) {
    n_cells[d] = std::pow(2, params[d].n_octaves_);
    pixel_size[d] = cell_size / (size[d] / n_cells[d]);
    if (size[d] % n_cells[d] != 0) {
      std::string message = 
        "Texture size must be evenly divisible by number of cells\n";
      throw std::invalid_argument(message);
    }
  }
  
  // Generate a cube of random seed points
  std::vector<glm::vec3> seeds(n_cells[0] * n_cells[1] * n_cells[2]);
  for (std::size_t i = 0; i < n_cells[0]; ++i) {
    for (std::size_t j = 0; j < n_cells[1]; ++j) {
      for (std::size_t k = 0; k < n_cells[2]; ++k) {
        std::size_t n = n_cells[0] * n_cells[1] * k + n_cells[0] * j + i;
        for (int d = 0; d < 3; ++d) {
          float r = ((float) rand() / (RAND_MAX));
          seeds[n][d] += r * cell_size;
        }
      }
    }
  }
  
  // Generate the data by finding the closest seed point for each data point
  std::vector<float> data(size[0] * size[1] * size[2]);
  for (std::size_t i = 0; i < size[0]; ++i) {
    int ic = i / (size[0] / n_cells[0]);
    for (std::size_t j = 0; j < size[1]; ++j) {
      int jc = j / (size[1] / n_cells[1]);
      for (std::size_t k = 0; k < size[2]; ++k) {
        int kc = k / (size[2] / n_cells[2]);
        glm::vec3 cent = glm::vec3(i + 0.5, j + 0.5, k + 0.5) * pixel_size;
        // Find closest seed point
        glm::vec3 cell0;
        float dist_min = std::numeric_limits<float>::max();
        for (int ii = -1; ii <= 1; ++ii) {
          int icn = ic + ii;
          if (icn < 0) icn = n_cells[0] - 1;
          if (icn == (int)n_cells[0]) icn = 0;
          cell0[0] = cell_size * (ic + ii);
          for (int jj = -1; jj <= 1; ++jj) {
            int jcn = jc + jj;
            if (jcn < 0) jcn = n_cells[1] - 1;
            if (jcn == (int)n_cells[1]) jcn = 0;
            cell0[1] = cell_size * (jc + jj);
            for (int kk = -1; kk <= 1; ++kk) {
              int kcn = kc + kk;
              if (kcn < 0) kcn = n_cells[2] - 1;
              if (kcn == (int)n_cells[2]) kcn = 0;
              cell0[2] = cell_size * (kc + kk);
              std::size_t n = 
                n_cells[0] * n_cells[1] * kcn + n_cells[0] * jcn + icn;
              dist_min = 
                std::min(glm::distance(cent, cell0 + seeds[n]), dist_min);
            }
          }
        }
        std::size_t n = size[0] * size[1] * k + size[0] * j + i;
        data[n] = 1 - dist_min;
      }
    }
  }
  return data;
}
  
//****************************************************************************80
std::vector<float> NoiseCube::GeneratePerlinNoise(
    const std::array<unsigned,3>& size,
    const std::array<NoiseParams,3>& params) const {

}

} // End namespace TopFun
