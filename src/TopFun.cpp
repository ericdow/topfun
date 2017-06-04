#include <iostream>
#include <chrono>
#include <thread>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "utils/DebugOverlay.h"
#include "terrain/Terrain.h"
#include "terrain/Skybox.h"

using namespace TopFun;

void DrawScene(Terrain& terrain, Skybox& skybox);

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {1200, 800};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

// Set up objects that can be modified by input callbacks
GLfloat terrain_size = 1000.0f; 
glm::vec3 start_pos(terrain_size/2, 20.0f, terrain_size/2);
Camera camera(screen_size, start_pos);
DebugOverlay debug_overlay(screen_size);
CallBackWorld callback_world(camera, debug_overlay, screen_size);

GLfloat last_draw_time = 0.0f;
GLfloat delta_loop_time = 0.0f;
// Force loop to sleep until this amount of time has passed
GLfloat loop_lock_time = 1.0/120.0;
int main(int argc, char* argv[]) {
  
  // Set up remaining game objects (in main due to static members)
  Terrain terrain(terrain_size, 20);
  Skybox skybox;

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
        DrawScene(terrain, skybox);
      }
    }
    else {
      DrawScene(terrain, skybox);
    }

    // Sleep (if possible)
    GLfloat end_loop_time = glfwGetTime();
    std::chrono::duration<float> sleep_duration(loop_lock_time - 
        (end_loop_time - current_loop_time));
    if (sleep_duration > std::chrono::duration<float>(0)) {
      std::this_thread::sleep_for(sleep_duration);
    }
    
  } // End game loop

  GLEnvironment::TearDown();
  return 0;
}

void DrawScene(Terrain& terrain, Skybox& skybox) {
  // Compute frame time
  GLfloat current_draw_time = glfwGetTime();
  GLfloat delta_draw_time = current_draw_time - last_draw_time;
  last_draw_time = current_draw_time;
  
  // Clear the colorbuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  terrain.Draw(camera);
  skybox.Draw(camera);
  // Display the debug console last
  debug_overlay.Draw(camera, delta_loop_time, delta_draw_time);
      
  // Swap the buffers
  glfwSwapBuffers(window);
}
