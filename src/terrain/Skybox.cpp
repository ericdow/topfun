#include "SOIL.h"
#include "terrain/Skybox.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Skybox::Skybox() : shader_("shaders/skybox.vs", "shaders/skybox.frag") {
  
  GLfloat vertices[] = {
  // Positions          
  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  -1.0f,  1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f
  };

  // Setup skybox VAO
  GLuint VBO;
  glGenVertexArrays(1, &VAO_);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO_);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
      3*sizeof(GLfloat), (GLvoid*)0);
  glBindVertexArray(0);
  glDeleteBuffers(1, &VBO);

  // Load cubemap textures
  std::vector<const GLchar*> faces;
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayLeft2048.png");
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayRight2048.png");
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayUp2048.png");
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayDown2048.png");
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayFront2048.png");
  faces.push_back("../../../assets/textures/TropicalSunnyDay/TropicalSunnyDayBack2048.png");
  LoadCubemap(faces);
}

//****************************************************************************80
Skybox::~Skybox() {
  glDeleteVertexArrays(1, &VAO_);
}

//****************************************************************************80
void Skybox::Draw(Camera const& camera) {		
  // Change depth function so depth test passes when values are equal to 
  // depth buffer's content
  glDepthFunc(GL_LEQUAL);  
  shader_.Use();
  SetShaderData(camera);  
  glBindVertexArray(VAO_);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(shader_.GetProgram(), "skybox"), 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS); // Set depth function back to default
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Skybox::LoadCubemap(std::vector<const GLchar*> const& faces) {
  glGenTextures(1, &cubemap_texture_);

  int width,height;
  unsigned char* image;
  
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_);
  for(GLuint i = 0; i < faces.size(); i++) {
    image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 
        0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

//****************************************************************************80
void Skybox::SetShaderData(Camera const& camera) {
  // Remove any translation component of the view matrix	
  glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "view"), 1, 
      GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shader_.GetProgram(), "projection"), 
      1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));
}

} // End namespace TopFun
