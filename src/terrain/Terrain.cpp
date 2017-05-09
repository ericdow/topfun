#include "SOIL.h"
#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain() : shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  LoadTexture();
  // Set up perlin noise generator
  perlin_generator_.SetOctaveCount(1);
}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  
  // Data packing:
  // - 3 floats (position)
  // - 3 floats (normal)
  // - 2 floats (texture)
  int n_vert_attrib = 8;
  GLuint nvx(100), nvz(100);
  GLuint nex(nvx-1), nez(nvz-1);
  std::vector<GLfloat> vertices(n_vert_attrib*nvx*nvz, 0.0f);
  std::vector<GLuint> indices(6*nex*nez);

  // Vertex position and texture coordinates
  GLfloat dx(1.0f), dz(1.0f);
  for (GLuint i = 0; i < nvx; ++i) {
    for (GLuint j = 0; j < nvz; ++j) {
      GLuint offset = n_vert_attrib*(nvx*j + i);
      vertices[offset    ] = -dx*i;
      // vertices[offset + 1] = 
      //   -10+0.05*dx*nvx*sin(15*dx*i/nvx)*sin(15*dz*j/nvz);
      vertices[offset + 1] = GetHeight((GLfloat) i, (GLfloat) j);
      vertices[offset + 2] = -dz*j;
      // texture
      if (i % 2) {
        vertices[offset + 6] = 0.0;
      }
      else {
        vertices[offset + 6] = 1.0;
      }
      if (j % 2) {
        vertices[offset + 7] = 0.0;
      }
      else {
        vertices[offset + 7] = 1.0;
      }
    }
  }
 
  // Element defitions 
  /*
    v2   v2   v1
     o    o---o
     |\    \t1|
     | \    \ |
     |t0\    \|
     o---o    o
    v0   v1   v0
  */
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
  
  // Vertex normals (smoothed)
  for (GLuint i = 0; i < nex; ++i) {
    for (GLuint j = 0; j < nez; ++j) {
      GLuint ind_offset = 6*(nex*j + i);
      // first triangle in this face
      GLuint v0_ix = n_vert_attrib*indices[ind_offset    ];
      GLuint v1_ix = n_vert_attrib*indices[ind_offset + 1];
      GLuint v2_ix = n_vert_attrib*indices[ind_offset + 2];
      glm::vec3 v0(vertices[v0_ix],vertices[v0_ix+1],vertices[v0_ix+2]);
      glm::vec3 v1(vertices[v1_ix],vertices[v1_ix+1],vertices[v1_ix+2]);
      glm::vec3 v2(vertices[v2_ix],vertices[v2_ix+1],vertices[v2_ix+2]);
      glm::vec3 normal = cross(v1 - v0, v2 - v0);
      
      vertices[v0_ix + 3] += normal.x;
      vertices[v0_ix + 4] += normal.y;
      vertices[v0_ix + 5] += normal.z;
      
      vertices[v1_ix + 3] += normal.x;
      vertices[v1_ix + 4] += normal.y;
      vertices[v1_ix + 5] += normal.z;
      
      vertices[v2_ix + 3] += normal.x;
      vertices[v2_ix + 4] += normal.y;
      vertices[v2_ix + 5] += normal.z;

      // second triangle in this face
      v0_ix = n_vert_attrib*indices[ind_offset + 3];
      v1_ix = n_vert_attrib*indices[ind_offset + 4];
      v2_ix = n_vert_attrib*indices[ind_offset + 5];
      v0 = glm::vec3(vertices[v0_ix],vertices[v0_ix+1],vertices[v0_ix+2]);
      v1 = glm::vec3(vertices[v1_ix],vertices[v1_ix+1],vertices[v1_ix+2]);
      v2 = glm::vec3(vertices[v2_ix],vertices[v2_ix+1],vertices[v2_ix+2]);
      normal = cross(v1 - v0, v2 - v0);
      
      vertices[v0_ix + 3] += normal.x;
      vertices[v0_ix + 4] += normal.y;
      vertices[v0_ix + 5] += normal.z;
      
      vertices[v1_ix + 3] += normal.x;
      vertices[v1_ix + 4] += normal.y;
      vertices[v1_ix + 5] += normal.z;
      
      vertices[v2_ix + 3] += normal.x;
      vertices[v2_ix + 4] += normal.y;
      vertices[v2_ix + 5] += normal.z;
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

  GLint pos_loc = glGetAttribLocation(shader_.GetProgram(), "position");
  GLint norm_loc = glGetAttribLocation(shader_.GetProgram(), "normal");
  GLint tex_loc = glGetAttribLocation(shader_.GetProgram(), "texCoord");

  // Position attribute
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib * sizeof(GLfloat), (GLvoid*)0);
  // Normal attribute
  glEnableVertexAttribArray(norm_loc);
  glVertexAttribPointer(norm_loc, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib * sizeof(GLfloat), (GLvoid*)12);
  // Texture attribute
  glEnableVertexAttribArray(tex_loc);
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,
      n_vert_attrib * sizeof(GLfloat), (GLvoid*)24);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0); 
    
  // Send data to the shaders
  SetShaderData(camera);

  // Bind the texture data
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "grassTexture"), 0);
  
  // Activate shader
  shader_.Use();

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
GLfloat Terrain::GetHeight(GLfloat x, GLfloat z) const {
  return perlin_generator_.GetValue(x, 0.2, z);
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
  glUniform3f(matColorLoc, 1.0f, 1.0f, 1.0f);
  glUniform1f(matShininessLoc, 1.0f);

  // Set lighting uniforms
  GLint lightDirectionLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.direction");
  GLint lightAmbientLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.ambient");
  GLint lightDiffuseLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.diffuse");
  GLint lightSpecularLoc = glGetUniformLocation(shader_.GetProgram(), 
      "light.specular");
  // TODO move light direction definition to somewhere higher up
  glUniform3f(lightDirectionLoc, 0.0f, -1.0f, 0.0f);
  glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);
  glUniform3f(lightDiffuseLoc, 0.5f, 0.5f, 0.5f);
  glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
  
  // Set the camera position uniform
  glm::vec3 camera_pos = camera.GetPosition();
  GLint viewPosLoc = glGetUniformLocation(shader_.GetProgram(), "viewPos");
  glUniform3f(viewPosLoc, camera_pos.x, camera_pos.y, camera_pos.z);
}

//****************************************************************************80
void Terrain::LoadTexture() {
  // Load and create a texture 
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_); 
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
      GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
      GL_LINEAR_MIPMAP_LINEAR);
  // Load, create texture and generate mipmaps
  int width, height;
  unsigned char* image = SOIL_load_image(
      "../../../assets/textures/light-grass-texture.jpg", &width, &height, 0, 
      SOIL_LOAD_RGB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
      GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  // Clean up
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // End namespace TopFun
