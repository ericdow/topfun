#ifndef TERRAINTILE_H
#define TERRAINTILE_H

#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "utils/Camera.h"

namespace TopFun {

namespace noise {
namespace module {
class Perlin;
}
}

class TerrainTile {
 
 public:
  //**************************************************************************80
  //! \brief TerrainTile - Constructor for empty terrain object
  //**************************************************************************80
  TerrainTile();
  
  //**************************************************************************80
  //! \brief ~TerrainTile - Destructor
  //**************************************************************************80
  ~TerrainTile();
  
  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a some (x,y) location
  //**************************************************************************80
  static GLfloat GetHeight(GLfloat x, GLfloat z);

  //**************************************************************************80
  //! \brief Draw - Draws the terrain tile
  //**************************************************************************80
  void Draw(Camera const& camera);
  
  //**************************************************************************80
  //! \brief SetNeighborLoD - updates the current level of detail for all
  //! tiles surrounding this tile
  //! \param[in] lod_nesw - array of LoD values for neighboring tiles
  //**************************************************************************80
  inline void SetNeighborLoD(const std::array<unsigned int,4>& lod_nesw) {
    lod_nesw_ = lod_nesw;
  }

  //**************************************************************************80
  //! \brief BuildAllElem2Node - Precomputes all possible element-to-node
  //! connectivities for all possible tile LODs and surrounding tile LODs 
  //**************************************************************************80
  static void BuildAllElem2Node();

 private:
  Shader shader_;
  // TODO const noise::module::Perlin& perlin_generator_;
  static const unsigned int lod_max_ = 4; // higher is coarser
  unsigned int lod_; // current level of detail
  std::array<unsigned int,4> lod_nesw_; // current LoD for all neighbor tiles
  // Element-to-node connectivities for all possible combinations of tile LOD
  // and surrounding tile LODs
  static std::vector<GLuint> elem2node_all_;
  // Offsets for where each element-to-node connectivity begins inside of
  // elem2node_all_
  static std::vector<GLuint> elem2node_all_offsets_;

  //**************************************************************************80
  //! \brief GetElem2NodeOffsetIndex - TODO
  //**************************************************************************80
  static GLuint GetElem2NodeOffsetIndex(unsigned short lod, 
      unsigned short lod_n, unsigned short lod_e, unsigned short lod_s, 
      unsigned short lod_w);
  
};
} // End namespace TopFun

#endif
