#include <iostream>

#include <noise/module/perlin.h>

#include "terrain/TerrainTile.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
TerrainTile::TerrainTile() : 
  shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  
}

//****************************************************************************80
TerrainTile::~TerrainTile() {
}

//****************************************************************************80
void TerrainTile::Draw(Camera const& camera) {
  // Activate shader
  shader_.Use();
}

//****************************************************************************80
GLfloat TerrainTile::GetHeight(GLfloat x, GLfloat z) {
  // return perlin_generator_.GetValue(x/10, z/10, 0.5);
}

//****************************************************************************80
void TerrainTile::BuildAllElem2Node() {
  // Indices follow right-hand-rule is out of the page
  for (unsigned int c = 0; c < lod_max_; ++c) {
    GLuint edge_size = pow(2, lod_max_ - c);
    for (unsigned int n = 0; n <= c; ++n) {
      for (unsigned int e = 0; e <= c; ++e) {
        for (unsigned int s = 0; s <= c; ++s) {
          for (unsigned int w = 0; w <= c; ++w) {
            // Build the north edge
            // Build the east edge
            // Build the south edge
            // Build the west edge
            // Build the center
            if (c < lod_max_) {
              for (GLuint i = 1; i < edge_size - 1; ++i) {
                for (GLuint j = 1; j < edge_size - 1; ++j) {
                  // Set diagonal edge orientation
                  if ((i <  edge_size/2 && j <  edge_size/2) ||
                      (i >= edge_size/2 && j >= edge_size/2)) {

                  }
                  else {

                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80

} // End namespace TopFun
