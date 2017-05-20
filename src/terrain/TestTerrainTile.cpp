#include <iostream>
#include "terrain/TerrainTile.h"
#include "utils/GLEnvironment.h"
#include <boost/unordered_map.hpp>

using namespace TopFun;

int main(int argc, char* argv[]) {

  // Set up the tiles
  TerrainTile::SetTileLength(10.0);
  
  return 0;
}

