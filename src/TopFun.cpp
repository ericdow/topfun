#include <iostream>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "utils/DebugOverlay.h"
#include "terrain/Terrain.h"

int main(int argc, char* argv[]) {
  using namespace TopFun;
  
  // Set up the GL/GLFW environment
  const std::array<GLuint,2> screen_size = {1200, 800};
  GLFWwindow* window = GLEnvironment::SetUp(screen_size);

  // Set up objects that can be modified by input callbacks 
  glm::vec3 start_pos(0.0f, 0.0f, 0.0f);
  Camera camera(screen_size, start_pos);
  DebugOverlay debug_overlay(screen_size);
  CallBackWorld callback_world(camera, debug_overlay, screen_size);
  
  GLEnvironment::SetCallback(window, callback_world);
  
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
    camera.Move(callback_world.GetKeyState(), delta_time);

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Draw the scene 
    terrain.Draw(camera);
  
    // Display the debug console (if visible)
    debug_overlay.Draw(camera, delta_time);

    // Swap the buffers
    glfwSwapBuffers(window);
  }

  GLEnvironment::TearDown();
  return 0;
}
