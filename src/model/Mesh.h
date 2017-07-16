#ifndef MESH_H
#define MESH_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>

#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// assimp includes
#include "types.h"

#include "shaders/Shader.h"

namespace TopFun {

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
};

struct Texture {
  GLuint id;
  std::string type;
  aiString path;
};

class Mesh {
 public:
  // Constructor
  Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, 
      std::vector<Texture> textures) : vertices_(vertices), indices_(indices),
        textures_(textures) {
    SetupMesh();
  }

  // Render the mesh
  void Draw(const Shader& shader) {
    // Bind appropriate textures
    GLuint diffuseNr = 1;
    GLuint specularNr = 1;
    GLuint normalNr = 1;
    GLuint heightNr = 1;
    for (GLuint i = 0; i < textures_.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      // Retrieve texture number (the N in diffuse_textureN)
      std::stringstream ss;
      std::string number;
      std::string name = textures_[i].type;
      if (name == "texture_diffuse")
        ss << diffuseNr++; // Transfer GLuint to stream
      else if (name == "texture_specular")
        ss << specularNr++; // Transfer GLuint to stream
      else if (name == "texture_normal")
        ss << normalNr++; // Transfer GLuint to stream
      else if (name == "texture_height")
        ss << heightNr++; // Transfer GLuint to stream
      number = ss.str(); 
      // Now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(shader.GetProgram(), 
            (name + number).c_str()), i);
      // And finally bind the texture
      glBindTexture(GL_TEXTURE_2D, textures_[i].id);
    }
    
    // Also set each mesh's shininess property to a default value 
    glUniform1f(glGetUniformLocation(shader.GetProgram(), "material.shininess"),
        16.0f);

    // Draw mesh
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Always good practice to set everything back to defaults once configured
    for (GLuint i = 0; i < textures_.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  // Forms an AABB of mesh
  std::array<std::array<float,2>,3> FormAABB() const {
    std::array<std::array<float,2>,3> AABB;
    AABB[0] = {vertices_[0].Position.x, vertices_[0].Position.x};
    AABB[1] = {vertices_[0].Position.y, vertices_[0].Position.y};
    AABB[2] = {vertices_[0].Position.z, vertices_[0].Position.z};
    for (size_t i = 1; i < vertices_.size(); ++i) {
      AABB[0][0] = std::min(vertices_[i].Position.x, AABB[0][0]);
      AABB[0][1] = std::max(vertices_[i].Position.x, AABB[0][1]);
      AABB[1][0] = std::min(vertices_[i].Position.y, AABB[1][0]);
      AABB[1][1] = std::max(vertices_[i].Position.y, AABB[1][1]);
      AABB[2][0] = std::min(vertices_[i].Position.z, AABB[2][0]);
      AABB[2][1] = std::max(vertices_[i].Position.z, AABB[2][1]);
    }
    return AABB;
  }

  inline GLuint GetNumTextures() const { return textures_.size(); }

 private:
  std::vector<Vertex> vertices_;
  std::vector<GLuint> indices_;
  std::vector<Texture> textures_;

  GLuint VAO_, VBO_, EBO_;

  // Initializes all the buffer objects/arrays
  void SetupMesh() {
    // Create buffers/arrays
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);
    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), 
        vertices_.data(), GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
        indices_.data(), GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    // Vertex Normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
        (GLvoid*)offsetof(Vertex, Normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
        (GLvoid*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
  }
};

} // End namespace TopFun

#endif
