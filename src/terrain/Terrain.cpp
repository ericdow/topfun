#include "terrain/Terrain.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Terrain::Terrain() : shader_("shaders/terrain.vs", "shaders/terrain.frag") {}

//****************************************************************************80
void Terrain::Draw(Camera const& camera) {
  // Generate triangle data
  GLfloat vertices[] = {
       // Positions         // Normals
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  };
 
  // Set up a VBO with coordinate data 
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set up VAO
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 
      (GLvoid*)0);
  glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 
      (GLvoid*)0);
  glEnableVertexAttribArray(1);
  glBindVertexArray(0); // Unbind VAO
    
  // Activate shader
  shader_.Use();

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

  // Render
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  
  // Clean up
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

} // End namespace TopFun
