#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain() : shader_("shaders/terrain.vs", "shaders/terrain.frag") {}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  
  // Generate triangle data
  int n_vert_attrib = 6; // positions, normals, texture coords, etc...
  GLuint nvx(10), nvz(10);
  GLuint nex(nvx-1), nez(nvz-1);
  std::vector<GLfloat> vertices(n_vert_attrib*nvx*nvz);
  std::vector<GLuint> indices(6*nex*nez);

  // Vertex coordinates
  GLfloat dx(1.0f), dz(1.0f);
  for (GLuint i = 0; i < nvx; ++i) {
    for (GLuint j = 0; j < nvz; ++j) {
      GLuint offset = n_vert_attrib*(nvx*j + i);
      vertices[offset    ] = dx*i;
      vertices[offset + 1] = 0.0f;
      vertices[offset + 2] = dz*j;
    }
  }

  // Vertex normals
  for (GLuint i = 0; i < nvx; ++i) {
    for (GLuint j = 0; j < nvz; ++j) {
      GLuint offset = n_vert_attrib*(nvx*j + i) + 3;
      vertices[offset    ] = dx*i;
      vertices[offset + 1] = 0.0f;
      vertices[offset + 2] = dz*j;
    }
  }

  for (GLuint i = 0; i < nex; ++i) {
    for (GLuint j = 0; j < nez; ++j) {
      GLuint offset = 6*(nex*j + i);
      // first triangle in this face
      indices[offset    ] = nvx*j + i;
      indices[offset + 1] = nvx*j + i + 1;
      indices[offset + 2] = nvx*(j + 1) + i;
      // second triangle in this face
      indices[offset + 3] = nvx*j + i + 1;
      indices[offset + 4] = nvx*(j + 1) + i + 1;
      indices[offset + 5] = nvx*(j + 1) + i;
    }
  }

  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), 
      vertices.data(), GL_STATIC_DRAW); // TODO static or dynamic???
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), 
      indices.data(), GL_STATIC_DRAW); //  TODO static or dynamic???

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  // Normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(1);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0); 
    
  // Activate shader
  shader_.Use();

  // Send data to the shaders
  SetShaderData(camera);
  
  // Render
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // Clean up
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO); 
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Terrain::SetShaderData(Camera const& camera) {
  // Set model/view/projection uniforms  
  glm::mat4 model; // TODO this is all zeros
  GLint modelLoc = glGetUniformLocation(shader_.GetProgram(), "model");
  GLint viewLoc = glGetUniformLocation(shader_.GetProgram(), "view");
  GLint projLoc = glGetUniformLocation(shader_.GetProgram(), "projection");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, 
      glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, 
      glm::value_ptr(camera.GetProjectionMatrix()));
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  // Set material uniforms
  GLint matColorLoc = glGetUniformLocation(shader_.GetProgram(), 
      "material.color");
  GLint matShininessLoc = glGetUniformLocation(shader_.GetProgram(), 
      "material.shininess");
  glUniform3f(matColorLoc, 0.3f, 1.0f, 0.3f);
  glUniform1f(matShininessLoc,  64.0f);

  // Set lighting uniforms
  GLint lightDirectionLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.direction");
  GLint lightAmbientLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.ambient");
  GLint lightDiffuseLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.diffuse");
  GLint lightSpecularLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.specular");
  glUniform3f(lightDirectionLoc, -1.0f, -1.0f, 0.0f);
  glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);
  glUniform3f(lightDiffuseLoc, 0.5f, 0.5f, 0.5f);
  glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
  
  // Set the camera position uniform
  glm::vec3 camera_pos = camera.GetPosition();
  GLint viewPosLoc = glGetUniformLocation(shader_.GetProgram(), "viewPos");
  glUniform3f(viewPosLoc, camera_pos.x, camera_pos.y, camera_pos.z);
}

} // End namespace TopFun
