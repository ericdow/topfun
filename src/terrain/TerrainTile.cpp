#include <iostream>
#include <array>

#include <noise/module/perlin.h>

#include "terrain/TerrainTile.h"

namespace TopFun {

//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
std::vector<GLuint> TerrainTile::elem2node_all_;
boost::unordered_map<NeighborLoD, GLuint> 
    TerrainTile::elem2node_all_offsets_;

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
  GLuint offset = 0;
  // Indices follow right-hand-rule is out of the page
  GLuint edge_size0 = std::pow(2, num_lod_);
  for (unsigned short c = 0; c < num_lod_; ++c) {
    GLuint edge_size = std::pow(2, num_lod_ - c);
    GLuint pow2c = std::pow(2,c);
    for (unsigned short n = 0; n <= c; ++n) {
      for (unsigned short e = 0; e <= c; ++e) {
        for (unsigned short s = 0; s <= c; ++s) {
          for (unsigned short w = 0; w <= c; ++w) {
            NeighborLoD neigh_lod(c, n, e, s, w);
            elem2node_all_offsets_[neigh_lod] = offset;
            // Build the north edge
            unsigned short num_edge_splits = c - n + 1;
            GLuint df  = pow2c / (num_edge_splits);
            for (GLuint i = 0; i < edge_size; ++i) {
              GLuint j = edge_size - 1;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              if (i < edge_size/2) {
                // Add lower triangle
                if (i > 0) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*j0 + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add upper (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+1) + i0 + df*(f + 1), 
                                     (edge_size0+1)*(j0+1) + i0 + df*f,
                                     (edge_size0+1)*j0 + i0 + 1, offset);
                }
              }
              else {
                // Add lower triangle
                if (i < edge_size - 1) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*j0 + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0 + 1, offset);
                }
                // Add upper (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+1) + i0 + df*(f + 1), 
                                     (edge_size0+1)*(j0+1) + i0 + df*f,
                                     (edge_size0+1)*j0 + i0, offset);
                }
              }
            }
            // Build the east edge
            num_edge_splits = c - e + 1;
            df = pow2c / (num_edge_splits);
            for (GLuint j = 0; j < edge_size; ++j) {
              GLuint i = edge_size - 1;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              if (j < edge_size/2) {
                // Add left triangle
                if (j > 0) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*j0 + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add right (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+df*f) + i0 + 1, 
                                     (edge_size0+1)*(j0+df*(f+1)) + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
              }
              else {
                // Add left triangle
                if (j < edge_size - 1) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*(j0+1) + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add right (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+df*f) + i0 + 1, 
                                     (edge_size0+1)*(j0+df*(f+1)) + i0 + 1,
                                     (edge_size0+1)*j0 + i0, offset);
                }
              }
            }
            // Build the south edge
            num_edge_splits = c - s + 1;
            df  = pow2c / (num_edge_splits);
            for (GLuint i = 0; i < edge_size; ++i) {
              GLuint j = 0;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              if (i < edge_size/2) {
                // Add upper triangle
                if (i > 0) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*(j0+1) + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add lower (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0 + df*f, 
                                     (edge_size0+1)*j0 + i0 + df*(f + 1),
                                     (edge_size0+1)*(j0+1) + i0 + 1, offset);
                }
              }
              else {
                // Add upper triangle
                if (i < edge_size - 1) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0 + 1, 
                                     (edge_size0+1)*(j0+1) + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add lower (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0 + df*f, 
                                     (edge_size0+1)*j0 + i0 + df*(f + 1),
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
              }
            }
            // Build the west edge
            num_edge_splits = c - w + 1;
            df  = pow2c / (num_edge_splits);
            for (GLuint j = 0; j < edge_size; ++j) {
              GLuint i = 0;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              if (j < edge_size/2) {
                // Add right triangle
                if (j > 0) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                     (edge_size0+1)*j0 + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0 + 1, offset);
                }
                // Add left (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+df*f) + i0, 
                                     (edge_size0+1)*(j0+1) + i0 + 1,
                                     (edge_size0+1)*(j0+df*(f+1)) + i0, offset);
                }
              }
              else {
                // Add right triangle
                if (j < edge_size - 1) {
                  AddTriangleIndices((edge_size0+1)*j0 + i0 + 1, 
                                     (edge_size0+1)*(j0+1) + i0 + 1,
                                     (edge_size0+1)*(j0+1) + i0, offset);
                }
                // Add left (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  AddTriangleIndices((edge_size0+1)*(j0+df*f) + i0, 
                                     (edge_size0+1)*j0 + i0 + 1,
                                     (edge_size0+1)*(j0+df*(f+1)) + i0, offset);
                }
              }
            }
            // Build the center
            if (c < num_lod_ - 1) {
              for (GLuint i = 1; i < edge_size - 1; ++i) {
                for (GLuint j = 1; j < edge_size - 1; ++j) {
                  GLuint i0 = i*pow2c;
                  GLuint j0 = j*pow2c;
                  // Set diagonal edge orientation
                  if ((i <  edge_size/2 && j <  edge_size/2) ||
                      (i >= edge_size/2 && j >= edge_size/2)) {
                    AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                       (edge_size0+1)*(j0+1) + i0 + 1,
                                       (edge_size0+1)*(j0+1) + i0, offset);
                    AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                       (edge_size0+1)*j0 + i0 + 1,
                                       (edge_size0+1)*(j0+1) + i0 + 1, offset);
                  }
                  else {
                    AddTriangleIndices((edge_size0+1)*j0 + i0, 
                                       (edge_size0+1)*j0 + i0 + 1,
                                       (edge_size0+1)*(j0+1) + i0, offset);
                    AddTriangleIndices((edge_size0+1)*j0 + i0 + 1, 
                                       (edge_size0+1)*(j0+1) + i0 + 1,
                                       (edge_size0+1)*(j0+1) + i0, offset);
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
void TerrainTile::AddTriangleIndices(GLuint i0, GLuint i1, GLuint i2, 
    GLuint& offset) {
  elem2node_all_.push_back(i0);
  elem2node_all_.push_back(i1);
  elem2node_all_.push_back(i2);
  offset += 3;
}

} // End namespace TopFun
