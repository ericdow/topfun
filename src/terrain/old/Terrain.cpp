#include "SOIL.h"
#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain(GLuint nvx, GLuint nvz, GLfloat lx, GLfloat lz) : 
  nvx_(nvx), nvz_(nvz), lx_(lx), lz_(lz),
  shader_("shaders/terrain.vs", "shaders/terrain.frag") {
  
  vertices_.resize(n_vert_attrib_*nvx_*nvz_);
  indices_.resize(6*(nvx_ - 1)*(nvz_ - 1));
  LoadTexture();
  
  // Set up perlin noise generator
  perlin_generator_.SetOctaveCount(7);
  perlin_generator_.SetFrequency(0.3);
  perlin_generator_.SetPersistence(0.5);

  // Vertex position and texture coordinates
  GLuint nex(nvx_-1), nez(nvz_-1);
  GLfloat dx = lx_/(nvx_ - 1);
  GLfloat dz = lz_/(nvz_ - 1);
  for (GLuint i = 0; i < nvx_; ++i) {
    for (GLuint j = 0; j < nvz_; ++j) {
      GLuint offset = n_vert_attrib_*(nvx_*j + i);
      vertices_[offset    ] = -dx*i;
      vertices_[offset + 1] = 3.0*GetHeight(dx*i, dx*j) - 10.0;
      vertices_[offset + 2] = -dz*j;
      // texture
      if (i % 2) {
        vertices_[offset + 6] = 0.0;
      }
      else {
        vertices_[offset + 6] = 1.0;
      }
      if (j % 2) {
        vertices_[offset + 7] = 0.0;
      }
      else {
        vertices_[offset + 7] = 1.0;
      }
    }
  }
 
  // Element definitions 
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
      indices_[offset    ] = nvx_*j + i;
      indices_[offset + 1] = nvx_*j + i + 1;
      indices_[offset + 2] = nvx_*(j + 1) + i;
      // second triangle in this face
      indices_[offset + 3] = nvx_*j + i + 1;
      indices_[offset + 4] = nvx_*(j + 1) + i + 1;
      indices_[offset + 5] = nvx_*(j + 1) + i;
    }
  }
  
  // Vertex normals (smoothed)
  for (GLuint i = 0; i < nex; ++i) {
    for (GLuint j = 0; j < nez; ++j) {
      GLuint ind_offset = 6*(nex*j + i);
      // first triangle in this face
      GLuint v0_ix = n_vert_attrib_*indices_[ind_offset    ];
      GLuint v1_ix = n_vert_attrib_*indices_[ind_offset + 1];
      GLuint v2_ix = n_vert_attrib_*indices_[ind_offset + 2];
      glm::vec3 v0(vertices_[v0_ix],vertices_[v0_ix+1],vertices_[v0_ix+2]);
      glm::vec3 v1(vertices_[v1_ix],vertices_[v1_ix+1],vertices_[v1_ix+2]);
      glm::vec3 v2(vertices_[v2_ix],vertices_[v2_ix+1],vertices_[v2_ix+2]);
      glm::vec3 normal = cross(v1 - v0, v2 - v0);
      
      vertices_[v0_ix + 3] += normal.x;
      vertices_[v0_ix + 4] += normal.y;
      vertices_[v0_ix + 5] += normal.z;
      
      vertices_[v1_ix + 3] += normal.x;
      vertices_[v1_ix + 4] += normal.y;
      vertices_[v1_ix + 5] += normal.z;
      
      vertices_[v2_ix + 3] += normal.x;
      vertices_[v2_ix + 4] += normal.y;
      vertices_[v2_ix + 5] += normal.z;

      // second triangle in this face
      v0_ix = n_vert_attrib_*indices_[ind_offset + 3];
      v1_ix = n_vert_attrib_*indices_[ind_offset + 4];
      v2_ix = n_vert_attrib_*indices_[ind_offset + 5];
      v0 = glm::vec3(vertices_[v0_ix],vertices_[v0_ix+1],vertices_[v0_ix+2]);
      v1 = glm::vec3(vertices_[v1_ix],vertices_[v1_ix+1],vertices_[v1_ix+2]);
      v2 = glm::vec3(vertices_[v2_ix],vertices_[v2_ix+1],vertices_[v2_ix+2]);
      normal = cross(v1 - v0, v2 - v0);
      
      vertices_[v0_ix + 3] += normal.x;
      vertices_[v0_ix + 4] += normal.y;
      vertices_[v0_ix + 5] += normal.z;
      
      vertices_[v1_ix + 3] += normal.x;
      vertices_[v1_ix + 4] += normal.y;
      vertices_[v1_ix + 5] += normal.z;
      
      vertices_[v2_ix + 3] += normal.x;
      vertices_[v2_ix + 4] += normal.y;
      vertices_[v2_ix + 5] += normal.z;
    }
  }
  
  // Set up VAO
  GLuint VBO, EBO;
  glGenVertexArrays(1, &VAO_);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(VAO_);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices_.size(), 
      vertices_.data(), GL_STATIC_DRAW); // TODO static or dynamic???
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices_.size(), 
      indices_.data(), GL_STATIC_DRAW); //  TODO static or dynamic???

  GLint pos_loc = glGetAttribLocation(shader_.GetProgram(), "position");
  GLint norm_loc = glGetAttribLocation(shader_.GetProgram(), "normal");
  GLint tex_loc = glGetAttribLocation(shader_.GetProgram(), "texCoord");

  // Position attribute
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib_ * sizeof(GLfloat), (GLvoid*)0);
  // Normal attribute
  glEnableVertexAttribArray(norm_loc);
  glVertexAttribPointer(norm_loc, 3, GL_FLOAT, GL_FALSE,
      n_vert_attrib_ * sizeof(GLfloat), (GLvoid*)12);
  // Texture attribute
  glEnableVertexAttribArray(tex_loc);
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,
      n_vert_attrib_ * sizeof(GLfloat), (GLvoid*)24);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0); 
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO); 
}

//****************************************************************************80
Terrain::~Terrain() {
  glDeleteVertexArrays(1, &VAO_);
}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  // Activate shader
  shader_.Use();

  // Send data to the shaders
  SetShaderData(camera);

  // Bind the texture data
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "grassTexture"), 0);
  
  // Render
  glBindVertexArray(VAO_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

//****************************************************************************80
GLfloat Terrain::GetHeight(GLfloat x, GLfloat z) const {
  return perlin_generator_.GetValue(x/10, z/10, 0.5);
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Terrain::SetShaderData(Camera const& camera) {
  // Set view/projection uniforms  
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"),
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

  // Set material uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), 
        "material.color"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), 
        "material.shininess"), 1.0f);

  // Set lighting uniforms
  // TODO move light direction definition to somewhere higher up
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.direction"),
      -0.3f, -1.0f, 0.0f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.ambient"), 
      0.2f, 0.2f, 0.2f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.diffuse"), 
      0.5f, 0.5f, 0.5f);
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "light.specular"), 
      1.0f, 1.0f, 1.0f);

  // Set fog uniforms
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "fog.Color"),
      183.0/256.0, 213.0/256.0, 219.0/256.0);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.Start"),
      70.0f);
  glUniform1f(glGetUniformLocation(shader_.GetProgram(), "fog.End"),
      200.0f);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "fog.Equation"),
      0);
  
  // Set the camera position uniform
  glm::vec3 camera_pos = camera.GetPosition();
  glUniform3f(glGetUniformLocation(shader_.GetProgram(), "viewPos"), 
      camera_pos.x, camera_pos.y, camera_pos.z);
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
  // TODO give better path...
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