#include <iostream>
#include <limits>
#include <array>

#include "module/perlin.h"

#include "terrain/TerrainTile.h"
#include "terrain/Terrain.h"

namespace TopFun {

//****************************************************************************80
// STATIC MEMBERS
//****************************************************************************80
GLfloat TerrainTile::l_tile_;
boost::unordered_map<NeighborLoD, std::vector<GLuint>> 
    TerrainTile::elem2node_all_ = BuildAllElem2Node();

//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
TerrainTile::TerrainTile(const Shader& shader, GLfloat x0, GLfloat z0) : 
  lods_(0,0,0,0,0), lods_prev_(0,0,0,0,0),
  neighbor_tiles_({nullptr, nullptr, nullptr, nullptr}) {

  // Set up vertices and normals
  std::vector<Vertex> vertices = SetupVertices(x0, z0);

  // Construct the bounding box
  // TODO  
 
  // Set up attribute and buffer objects
  GLuint VBO;
  glGenVertexArrays(1, &VAO_);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO_);
  
  glBindVertexArray(VAO_);

  // Set up the VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), 
      vertices.data(), GL_STATIC_DRAW);
  
  // Set up the initial EBO
  pelem2node_ = &elem2node_all_[lods_];
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * pelem2node_->size(), 
      pelem2node_->data(), GL_DYNAMIC_DRAW);

  GLint pos_loc  = glGetAttribLocation(shader.GetProgram(), "position");
  GLint norm_loc = glGetAttribLocation(shader.GetProgram(), "normal");
  GLint tex_loc  = glGetAttribLocation(shader.GetProgram(), "texCoord");
 
  // Position attribute
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
      reinterpret_cast<GLvoid*>(offsetof(Vertex, position)));
  // Normal attribute
  glEnableVertexAttribArray(norm_loc);
  glVertexAttribPointer(norm_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
      reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));
  // Texture attribute
  glEnableVertexAttribArray(tex_loc);
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
      reinterpret_cast<GLvoid*>(offsetof(Vertex, texture)));

  // Unbind VBO and VAO, but not EBO
  // The call to glVertexAttribPointer registers VBO to VAO, so safe to unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
}

//****************************************************************************80
TerrainTile::~TerrainTile() {
  glDeleteBuffers(1, &EBO_); 
  glDeleteVertexArrays(1, &VAO_);
}

//****************************************************************************80
void TerrainTile::Draw(Camera const& camera) {
  // Determine if any vertices of the AABB for this tile are in camera frustrum
  // TODO
 
  // Update element-to-node connectivity if this tile or neighbor LoD changed
  UpdateNeighborLoD();
  if (lods_prev_ != lods_) {
    UpdateElem2Node();
    lods_prev_ = lods_;
  }
  
  // Render
  glBindVertexArray(VAO_);
  glDrawElements(GL_TRIANGLES, pelem2node_->size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
  
//****************************************************************************80
std::vector<TerrainTile::Vertex> TerrainTile::SetupVertices(GLfloat x0, 
    GLfloat z0) {
  // Generate one layer of halo elements to smooth normals with
  int ne = std::pow(2,num_lod_);
  int nv = ne+1;
  std::vector<Vertex> vertices(std::pow(nv+2,2));

  // Vertex position
  ymin_ = std::numeric_limits<GLfloat>::max();
  ymax_ = std::numeric_limits<GLfloat>::min();
  GLfloat dx = l_tile_/ne;
  for (int i = 0; i < nv+2; ++i) {
    for (int j = 0; j < nv+2; ++j) {
      GLuint ix = (nv+2)*j + i;
      vertices[ix].position[0] = x0 + dx*(i-1);
      vertices[ix].position[1] = 3.0*Terrain::GetHeight(x0 + dx*(i-1), 
          z0 + dx*(j-1));
      vertices[ix].position[2] = z0 + dx*(j-1);
      // Zero out the normals
      for (int d = 0; d < 3; ++d) {
        vertices[ix].normal[d] = 0.0;
      }
    }
  }
 
  // Vertex normals (smoothed)
  for (int i = 0; i < ne+2; ++i) {
    for (int j = 0; j < ne+2; ++j) {
      GLuint v0_ix = (nv+2)* j +    i    ;
      GLuint v1_ix = (nv+2)* j +    i + 1;
      GLuint v2_ix = (nv+2)*(j+1) + i + 1;
      GLuint v3_ix = (nv+2)*(j+1) + i    ;
      glm::vec3 e01(vertices[v1_ix].position[0] - vertices[v0_ix].position[0],
                    vertices[v1_ix].position[1] - vertices[v0_ix].position[1],
                    vertices[v1_ix].position[2] - vertices[v0_ix].position[2]);
      glm::vec3 e12(vertices[v2_ix].position[0] - vertices[v1_ix].position[0],
                    vertices[v2_ix].position[1] - vertices[v1_ix].position[1],
                    vertices[v2_ix].position[2] - vertices[v1_ix].position[2]);
      glm::vec3 e23(vertices[v3_ix].position[0] - vertices[v2_ix].position[0],
                    vertices[v3_ix].position[1] - vertices[v2_ix].position[1],
                    vertices[v3_ix].position[2] - vertices[v2_ix].position[2]);
      glm::vec3 e30(vertices[v0_ix].position[0] - vertices[v3_ix].position[0],
                    vertices[v0_ix].position[1] - vertices[v3_ix].position[1],
                    vertices[v0_ix].position[2] - vertices[v3_ix].position[2]);
      glm::vec3 e02(vertices[v2_ix].position[0] - vertices[v0_ix].position[0],
                    vertices[v2_ix].position[1] - vertices[v0_ix].position[1],
                    vertices[v2_ix].position[2] - vertices[v0_ix].position[2]);
      glm::vec3 e13(vertices[v3_ix].position[0] - vertices[v1_ix].position[0],
                    vertices[v3_ix].position[1] - vertices[v1_ix].position[1],
                    vertices[v3_ix].position[2] - vertices[v1_ix].position[2]);
      glm::vec3 n0 = cross(e01,  e02) + cross( e02, -e30);
      glm::vec3 n1 = cross(e12,  e13) + cross( e13, -e01);
      glm::vec3 n2 = cross(e23, -e02) + cross(-e02, -e12);
      glm::vec3 n3 = cross(e30, -e13) + cross(-e13, -e23);
      for (int d = 0; d < 3; ++d) {
        vertices[v0_ix].normal[d] += n0[d];
        vertices[v1_ix].normal[d] += n1[d];
        vertices[v2_ix].normal[d] += n2[d];
        vertices[v3_ix].normal[d] += n3[d];
      }
    }
  }

  // Copy interior data out
  std::vector<Vertex> vertices_out(std::pow(nv,2));
  ymin_ = std::numeric_limits<GLfloat>::max();
  ymax_ = std::numeric_limits<GLfloat>::min();
  for (int i = 0; i < nv; ++i) {
    for (int j = 0; j < nv; ++j) {
      GLuint ix = nv*j + i;
      GLuint ix0 = (nv+2)*(j+1) + i + 1;
      for (int d = 0; d < 3; ++d) {
        vertices_out[ix].position[d] = vertices[ix0].position[d];
        vertices_out[ix].normal[d]   = vertices[ix0].normal[d];
      }
      // Set the bounding box
      ymin_ = std::min(ymin_, vertices_out[ix].position[1]);
      ymax_ = std::max(ymax_, vertices_out[ix].position[1]);
    }
  }

  // Set the texture coordinates based on largest tile size
  GLuint nrepeat = 40; // number of texture repetitions per largest tile
  GLuint denom = std::pow(2, num_lod_ - nrepeat + 1);
  denom = std::max(denom, 1u);
  for (int i = 0; i < nv; ++i) {
    for (int j = 0; j < nv; ++j) {
      GLuint ix = nv*j + i;
      vertices_out[ix].texture[0] = (GLfloat) i / denom;
      vertices_out[ix].texture[1] = (GLfloat) j / denom;
    }
  }
  
  // Set the centroid
  centroid_[0] = x0 + l_tile_/2;
  centroid_[1] = (ymin_ + ymax_)/2;
  centroid_[2] = z0 + l_tile_/2;

  return vertices_out;
}
  
//****************************************************************************80
void TerrainTile::UpdateNeighborLoD() {
  // If neighbor doesn't exist, set neighbor LoD to self LoD 
  if (neighbor_tiles_[0] != nullptr) {
    lods_.tuple.get<1>() = (neighbor_tiles_[0])->GetLoD();
  }
  else {
    lods_.tuple.get<1>() = lods_.tuple.get<0>();
  }
  if (neighbor_tiles_[1] != nullptr) {
    lods_.tuple.get<2>() = (neighbor_tiles_[1])->GetLoD();
  }
  else {
    lods_.tuple.get<2>() = lods_.tuple.get<0>();
  }
  if (neighbor_tiles_[2] != nullptr) {
    lods_.tuple.get<3>() = (neighbor_tiles_[2])->GetLoD();
  }
  else {
    lods_.tuple.get<3>() = lods_.tuple.get<0>();
  }
  if (neighbor_tiles_[3] != nullptr) {
    lods_.tuple.get<4>() = (neighbor_tiles_[3])->GetLoD();
  }
  else {
    lods_.tuple.get<4>() = lods_.tuple.get<0>();
  }
}

//****************************************************************************80
void TerrainTile::UpdateElem2Node() {
  // Ignore coarser neighbor tiles
  NeighborLoD lods = lods_;
  lods.tuple.get<1>() = std::min(lods_.tuple.get<1>(), lods_.tuple.get<0>());
  lods.tuple.get<2>() = std::min(lods_.tuple.get<2>(), lods_.tuple.get<0>());
  lods.tuple.get<3>() = std::min(lods_.tuple.get<3>(), lods_.tuple.get<0>());
  lods.tuple.get<4>() = std::min(lods_.tuple.get<4>(), lods_.tuple.get<0>());
  pelem2node_ = &elem2node_all_[lods];
  // Update the buffer object with new element indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
  void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  std::memcpy(ptr, pelem2node_->data(), sizeof(GLuint) * pelem2node_->size());
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

//****************************************************************************80
void TerrainTile::SetTileLength(GLfloat l_tile) {
  l_tile_ = l_tile;
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
boost::unordered_map<NeighborLoD, std::vector<GLuint>>
TerrainTile::BuildAllElem2Node() {
  boost::unordered_map<NeighborLoD, std::vector<GLuint>> 
    elem2node_all;
  // Indices follow right-hand-rule is out of the page
  GLuint nv0 = std::pow(2, num_lod_) + 1;
  for (unsigned short c = 0; c < num_lod_; ++c) {
    GLuint edge_size = std::pow(2, num_lod_ - c);
    GLuint pow2c = std::pow(2,c);
    for (unsigned short n = 0; n <= c; ++n) {
      for (unsigned short e = 0; e <= c; ++e) {
        for (unsigned short s = 0; s <= c; ++s) {
          for (unsigned short w = 0; w <= c; ++w) {
            std::vector<GLuint> elem2node;
            // Build the north edge
            unsigned short num_edge_splits = std::pow(2,c-n);
            GLuint df  = pow2c / (num_edge_splits);
            for (GLuint i = 0; i < edge_size; ++i) {
              GLuint j = edge_size - 1;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              GLuint i0p = (i+1)*pow2c;
              GLuint j0p = (j+1)*pow2c;
              if (i < edge_size/2) {
                // Add lower triangle
                if (i > 0) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add upper (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*j0p + i0 + df*(f + 1));
                  elem2node.push_back(nv0*j0p + i0 + df*f);
                  elem2node.push_back(nv0*j0 + i0p);
                }
              }
              else {
                // Add lower triangle
                if (i < edge_size - 1) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*j0p + i0p);
                }
                // Add upper (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*j0p + i0 + df*(f + 1));
                  elem2node.push_back(nv0*j0p + i0 + df*f);
                  elem2node.push_back(nv0*j0 + i0);
                }
              }
            }
            // Build the east edge
            num_edge_splits = std::pow(2,c-e);
            df = pow2c / (num_edge_splits);
            for (GLuint j = 0; j < edge_size; ++j) {
              GLuint i = edge_size - 1;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              GLuint i0p = (i+1)*pow2c;
              GLuint j0p = (j+1)*pow2c;
              if (j < edge_size/2) {
                // Add left triangle
                if (j > 0) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add right (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*(j0+df*f) + i0p); 
                  elem2node.push_back(nv0*(j0+df*(f+1)) + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
              }
              else {
                // Add left triangle
                if (j < edge_size - 1) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0p + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add right (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*(j0+df*f) + i0p); 
                  elem2node.push_back(nv0*(j0+df*(f+1)) + i0p);
                  elem2node.push_back(nv0*j0 + i0);
                }
              }
            }
            // Build the south edge
            num_edge_splits = std::pow(2,c-s);
            df  = pow2c / (num_edge_splits);
            for (GLuint i = 0; i < edge_size; ++i) {
              GLuint j = 0;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              GLuint i0p = (i+1)*pow2c;
              GLuint j0p = (j+1)*pow2c;
              if (i < edge_size/2) {
                // Add upper triangle
                if (i > 0) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0p + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add lower (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*j0 + i0 + df*f); 
                  elem2node.push_back(nv0*j0 + i0 + df*(f + 1));
                  elem2node.push_back(nv0*j0p + i0p);
                }
              }
              else {
                // Add upper triangle
                if (i < edge_size - 1) {
                  elem2node.push_back(nv0*j0 + i0p); 
                  elem2node.push_back(nv0*j0p + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add lower (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*j0 + i0 + df*f); 
                  elem2node.push_back(nv0*j0 + i0 + df*(f + 1));
                  elem2node.push_back(nv0*j0p + i0);
                }
              }
            }
            // Build the west edge
            num_edge_splits = std::pow(2,c-w);
            df  = pow2c / (num_edge_splits);
            for (GLuint j = 0; j < edge_size; ++j) {
              GLuint i = 0;
              GLuint i0 = i*pow2c;
              GLuint j0 = j*pow2c;
              GLuint i0p = (i+1)*pow2c;
              GLuint j0p = (j+1)*pow2c;
              if (j < edge_size/2) {
                // Add right triangle
                if (j > 0) {
                  elem2node.push_back(nv0*j0 + i0); 
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*j0p + i0p);
                }
                // Add left (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*(j0+df*f) + i0); 
                  elem2node.push_back(nv0*j0p + i0p);
                  elem2node.push_back(nv0*(j0+df*(f+1)) + i0);
                }
              }
              else {
                // Add right triangle
                if (j < edge_size - 1) {
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*j0p + i0p);
                  elem2node.push_back(nv0*j0p + i0);
                }
                // Add left (split) triangles
                for (GLuint f = 0; f < num_edge_splits; ++f) {
                  elem2node.push_back(nv0*(j0+df*f) + i0);
                  elem2node.push_back(nv0*j0 + i0p);
                  elem2node.push_back(nv0*(j0+df*(f+1)) + i0);
                }
              }
            }
            // Build the center
            if (c < num_lod_ - 1) {
              for (GLuint i = 1; i < edge_size - 1; ++i) {
                for (GLuint j = 1; j < edge_size - 1; ++j) {
                  GLuint i0 = i*pow2c;
                  GLuint j0 = j*pow2c;
                  GLuint i0p = (i+1)*pow2c;
                  GLuint j0p = (j+1)*pow2c;
                  // Set diagonal edge orientation
                  if ((i <  edge_size/2 && j <  edge_size/2) ||
                      (i >= edge_size/2 && j >= edge_size/2)) {
                    elem2node.push_back(nv0*j0 + i0);
                    elem2node.push_back(nv0*j0p + i0p);
                    elem2node.push_back(nv0*j0p + i0);
                    elem2node.push_back(nv0*j0 + i0);
                    elem2node.push_back(nv0*j0 + i0p);
                    elem2node.push_back(nv0*j0p + i0p);
                  }
                  else {
                    elem2node.push_back(nv0*j0 + i0);
                    elem2node.push_back(nv0*j0 + i0p);
                    elem2node.push_back(nv0*j0p + i0);
                    elem2node.push_back(nv0*j0 + i0p);
                    elem2node.push_back(nv0*j0p + i0p);
                    elem2node.push_back(nv0*j0p + i0);
                  }
                }
              }
            }
            NeighborLoD neigh_lod(c, n, e, s, w);
            elem2node_all[neigh_lod] = elem2node;
          }
        }
      }
    }
  }
  return elem2node_all;
}

} // End namespace TopFun
