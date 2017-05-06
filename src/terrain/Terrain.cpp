#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain() : shader_("shaders/terrain.vs", "shaders/terrain.frag") {
}

//****************************************************************************80
Terrain::~Terrain() { }

//****************************************************************************80
void Terrain::Draw() {
  // Generate triangle data

  // Render
}

} // End namespace TopFun
