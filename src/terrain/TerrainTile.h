#ifndef TERRAINTILE_H
#define TERRAINTILE_H

#include <vector>
#include <array>

#include <boost/unordered_map.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/Shader.h"
#include "render/Camera.h"
#include "terrain/NeighborLoD.h"

namespace TopFun {

class TerrainTile {

 public:
  //**************************************************************************80
  //! \brief TerrainTile - Constructor
  //**************************************************************************80
  TerrainTile(const Shader& shader, GLfloat x0, GLfloat z0);
  
  //**************************************************************************80
  //! \brief ~TerrainTile - Destructor
  //**************************************************************************80
  ~TerrainTile();

  //**************************************************************************80
  //! \brief Draw - Draws the terrain tile
  //**************************************************************************80
  void Draw();
  
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
  //! \brief UpdateLoD - sets the level of detail for this tile
  //**************************************************************************80
  inline void UpdateLoD(const glm::vec3& camera_pos) {
    GLfloat r_max = 10*l_tile_;
    GLfloat d_camera = std::sqrt(
        (camera_pos[0] - centroid_[0]) * (camera_pos[0] - centroid_[0]) +
        (camera_pos[2] - centroid_[2]) * (camera_pos[2] - centroid_[2]));
    GLuint lod = GLuint(d_camera / r_max * num_lod_);
    lods_.tuple.get<0>() = std::min(lod, GLuint(num_lod_ - 1));
  }

  //**************************************************************************80
  //! \brief UpdateNeighborLoD - updates the values of neighbor LoDs
  //**************************************************************************80
  void UpdateNeighborLoD();
  
  //**************************************************************************80
  //! \brief SetTileLength - sets the physical dimensions of the tiles
  //! \param[in] l_tile - physical dimension of the tile edges
  //**************************************************************************80
  static void SetTileLength(GLfloat l_tile);

  //**************************************************************************80
  //! \brief GetBoundingHeight - get the maximum height in this tile
  //**************************************************************************80
  inline float GetBoundingHeight() const { return ymax_; }

 private:
  GLuint VAO_, EBO_;
  static GLfloat l_tile_; // length of the tile edge
  glm::vec3 centroid_;
  GLfloat ymax_, ymin_; // for bounding box
  static const unsigned short num_lod_ = 6; // higher is coarser
  NeighborLoD lods_; // current level of detail of this tile and neighbors
  NeighborLoD lods_prev_; // level of detail on last Draw()
  // Pointers to NESW tiles, null if no neighbor exists
  std::array<const TerrainTile*,4> neighbor_tiles_;
  // Element-to-node connectivities for all possible combinations of tile LOD
  // and surrounding tile LODs
  static boost::unordered_map<NeighborLoD, std::vector<GLuint>> elem2node_all_;
  // Pointer to current elem2node for this tile
  std::vector<GLuint>* pelem2node_;
 
  // Helper struct for storing vertex attributes 
  struct Vertex {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texture[2];
  };

  //**************************************************************************80
  //! \brief SetupVertices - computes positions, normals, etc. 
  //**************************************************************************80
  std::vector<Vertex> SetupVertices(GLfloat x0, GLfloat z0);  

  //**************************************************************************80
  //! \brief UpdateElem2Node() - updates the element array buffer with the 
  //! current element-to-node connectivity based on neighbor's LoD values
  //**************************************************************************80
  void UpdateElem2Node();

  //**************************************************************************80
  //! \brief BuildAllElem2Node - precomputes all possible element-to-node
  //! connectivities for all possible tile LODs and surrounding tile LODs 
  //**************************************************************************80
  static boost::unordered_map<NeighborLoD, std::vector<GLuint>> 
    BuildAllElem2Node();
  
};
} // End namespace TopFun

#endif
