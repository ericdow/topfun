#include <iostream>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "utils/DebugOverlay.h"
#include "terrain/Terrain.h"
#include "terrain/Skybox.h"

using namespace TopFun;

void DrawScene();

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {1200, 800};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

// Set up objects that can be modified by input callbacks 
glm::vec3 start_pos(-250.0f, 0.0f, -250.0f);
Camera camera(screen_size, start_pos);
DebugOverlay debug_overlay(screen_size);
CallBackWorld callback_world(camera, debug_overlay, screen_size);

// Set up remaining game objects 
Terrain terrain(500, 500, 500.0f, 500.0f);
Skybox skybox;

GLfloat last_draw_time = 0.0f;
GLfloat delta_loop_time = 0.0f;
int main(int argc, char* argv[]) {

  // Point callback to correct location  
  GLEnvironment::SetCallback(window, callback_world);
  
  // Game loop
  GLfloat last_loop_time = 0.0f;
  GLfloat draw_wait_time = 0.0f;
  while(!glfwWindowShouldClose(window)) {
    // Compute loop time
    GLfloat current_loop_time = glfwGetTime();
    delta_loop_time = current_loop_time - last_loop_time;
    last_loop_time = current_loop_time;

    // Check and call events
    glfwPollEvents();
    camera.Move(callback_world.GetKeyState(), delta_loop_time);

    // Draw the scene
    draw_wait_time += delta_loop_time;
    if (callback_world.IsFPSLocked()) {
      if (draw_wait_time > 0.01666) {
        draw_wait_time = 0.0;
        DrawScene();
      }
    }
    else {
      DrawScene();
    }
  } // End game loop

  GLEnvironment::TearDown();
  return 0;
}

void DrawScene() {
  // Compute frame time
  GLfloat current_draw_time = glfwGetTime();
  GLfloat delta_draw_time = current_draw_time - last_draw_time;
  last_draw_time = current_draw_time;
  
  // Clear the colorbuffer
  glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  terrain.Draw(camera);
  skybox.Draw(camera, screen_size);
  // Display the debug console last
  debug_overlay.Draw(camera, delta_loop_time, delta_draw_time);
      
  // Swap the buffers
  glfwSwapBuffers(window);
}
