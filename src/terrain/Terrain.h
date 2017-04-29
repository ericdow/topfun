#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <array>

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
  ~Terrain();
  
  //**************************************************************************80
  //! \brief GetHeight - Get the terrain height at a bunch of (x,y) locations
  //**************************************************************************80
  std::vector<float> GetHeight(std::vector<std::array<float,2>> const& xy) 
    const;

 private:

};
} // End namespace TopFun

#endif
