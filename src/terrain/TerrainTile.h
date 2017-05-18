#ifndef TERRAINTILE_H
#define TERRAINTILE_H

#include <vector>
#include <array>

#include <boost/unordered_map.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "utils/Camera.h"
#include "terrain/NeighborLoD.h"

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
  //! \brief SetNeighborPointer - sets pointers to a neighbor tile
  //! \param[in] tile - pointer to a tile
  //! \param[in] ix - index of pointer to set (0:N, 1:E, 2:S, 3:W)
  //**************************************************************************80
  inline void SetNeighborPointer(const TerrainTile* tile, int ix) {
    neighbor_tiles_[ix] = tile;
  }
  
  //**************************************************************************80
  //! \brief GetLoD - returns the current level of detail for this tile
  //**************************************************************************80
  inline unsigned short GetLoD() const {
    return lods_.tuple.get<0>();
  }

  //**************************************************************************80
  //! \brief UpdateNeighborLoD - updates the values of neighbor LoDs
  //**************************************************************************80
  void UpdateNeighborLoD();

  //**************************************************************************80
  //! \brief BuildAllElem2Node - precomputes all possible element-to-node
  //! connectivities for all possible tile LODs and surrounding tile LODs 
  //**************************************************************************80
  static void BuildAllElem2Node();

  //**************************************************************************80
  //! \brief UpdateElem2Node() - updates the element array buffer with the 
  //! current element-to-node connectivity based on neighbor's LoD values
  //**************************************************************************80
  void UpdateElem2Node();

 private:
  Shader shader_;
  GLuint VAO_, EBO_;
  GLfloat ymax_, ymin_; // for bounding box
  // TODO const noise::module::Perlin& perlin_generator_;
  static const unsigned short num_lod_ = 4; // higher is coarser
  std::vector<GLfloat> y_;
  NeighborLoD lods_; // current level of detail of this tile and neighbors
  // Pointers to NESW tiles, null if no neighbor exists
  std::array<const TerrainTile*,4> neighbor_tiles_;
  // Element-to-node connectivities for all possible combinations of tile LOD
  // and surrounding tile LODs
  static std::vector<GLuint> elem2node_all_;
  // Offsets and sizes for where each element-to-node connectivity chunk begins 
  // and ends inside of elem2node_all_
  static boost::unordered_map<NeighborLoD, std::array<GLuint,2>> 
    elem2node_all_offsets_and_sizes_;

  //**************************************************************************80
  //! \brief GetElem2NodeOffsetIndex - TODO
  //**************************************************************************80
  static GLuint GetElem2NodeOffsetIndex(unsigned short lod, 
      unsigned short lod_n, unsigned short lod_e, unsigned short lod_s, 
      unsigned short lod_w);
  
  //**************************************************************************80
  //! \brief AddTriangleIndices - helper function to add a new triangle to
  //! elem2node_all_
  //**************************************************************************80
  static void AddTriangleIndices(GLuint i0, GLuint i1, GLuint i2, 
      GLuint& offset);
  
};
} // End namespace TopFun

#endif
