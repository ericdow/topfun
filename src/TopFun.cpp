#include <iostream>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "terrain/Terrain.h"

// TODO
#include "utils/TextRenderer.h"

int main(int argc, char* argv[]) {
  using namespace TopFun;

  // Set up objects that will be modified during input callbacks 
  GLuint width(1200), height(800);
  glm::vec3 start_pos(0.0f, 0.0f, 0.0f);
  Camera camera(width, height, start_pos);
  CallBackWorld call_back_world(camera, width, height);

  // Set up the GL/GLFW environment
  GLFWwindow* window = GLEnvironment::SetUp(width, height, call_back_world);
  
  // Set up remaining game objects 
  Terrain terrain(100, 100, 100.0f, 100.0f);

  // Game loop
  GLfloat delta_time = 0.0f;
  GLfloat last_frame_time = 0.0f;
  while(!glfwWindowShouldClose(window)) {
    // Set frame time
    GLfloat current_frame_time = glfwGetTime();
    delta_time = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    // Check and call events
    glfwPollEvents();
    camera.Move(call_back_world.GetKeyState(), delta_time);

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Draw the scene 
    terrain.Draw(camera);
  
    // Display the debug console (if visible)
    // TODO
    TextRenderer tr(width, height);
    std::string s = "Draw time: " + std::to_string(delta_time) + " s";
    tr.Draw(s, 10, 10, 0.7, glm::vec3(1.0,1.0,1.0));

    // Swap the buffers
    glfwSwapBuffers(window);
  }

  GLEnvironment::TearDown();
  return 0;
}
