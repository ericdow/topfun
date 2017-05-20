#include <iostream>
#include "terrain/TerrainTile.h"
#include "utils/GLEnvironment.h"

using namespace TopFun;

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {1200, 800};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

int main(int argc, char* argv[]) {

  // Set up the tiles
  TerrainTile::SetTileLength(1.0);

  Shader shader("../shaders/terrain.vs", "../shaders/terrain.frag");
  TerrainTile tt(shader);

  GLEnvironment::TearDown();
  return 0;
}

