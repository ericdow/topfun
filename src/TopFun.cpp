#include <iostream>
#include <chrono>
#include <thread>

#include <boost/numeric/odeint.hpp>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "utils/DebugOverlay.h"
#include "terrain/Terrain.h"
#include "terrain/Sky.h"
#include "aircraft/Aircraft.h"

using namespace TopFun;

void DrawScene(Terrain& terrain, Sky& sky, Aircraft& aircraft);

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {1200, 800};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

// Set up objects that can be modified by input callbacks
GLfloat terrain_size = 10000.0f; 
glm::vec3 start_pos(terrain_size/2, 20.0f, terrain_size/2);
Camera camera(screen_size, start_pos);
DebugOverlay debug_overlay(screen_size);
Aircraft aircraft(glm::vec3(terrain_size/2, 20.0f, terrain_size/2), 
    glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
CallBackWorld callback_world(camera, debug_overlay, screen_size);

GLfloat last_draw_time = 0.0f;
GLfloat dt_loop = 0.0f;
// Force loop to sleep until this amount of time has passed
GLfloat loop_lock_time = 1.0/120.0;
int main(int /* argc */, char** /* argv */) {
  
  // Set up remaining game objects (in main due to static members)
  Terrain terrain(terrain_size, 20);
  Sky sky;

  // Point callback to correct location  
  GLEnvironment::SetCallback(window, callback_world);

  // Set up ODE integrator
  boost::numeric::odeint::runge_kutta4<std::vector<float>> integrator;
  
  // Game loop
  GLfloat last_loop_time = 0.0f;
  GLfloat draw_wait_time = 0.0f;
  float t_physics = 0.0f;
  float dt_physics;
  while(!glfwWindowShouldClose(window)) {
    // Compute loop time
    GLfloat current_loop_time = glfwGetTime();
    dt_loop = current_loop_time - last_loop_time;
    dt_physics = dt_loop; // TODO
    last_loop_time = current_loop_time;

    // Check and call events
    glfwPollEvents();
   
    // Update the aircraft state 
    // aircraft.Move(callback_world.GetKeyState(), dt_loop);
    aircraft.UpdateControls(callback_world.GetKeyState());
    std::vector<float> state = aircraft.GetState();
    integrator.do_step(aircraft, state, t_physics, dt_physics);
    aircraft.SetState(state);
    
    // Update the camera position
    // camera.Move(callback_world.GetKeyState(), dt_loop);
    glm::vec3 aircraft_front = aircraft.GetFrontDirection();
    glm::vec3 aircraft_up = aircraft.GetUpDirection();
    camera.SetPosition(aircraft.GetPosition() + 
        2.0f * aircraft_up - 17.0f * aircraft_front);
    camera.SetOrientation(aircraft_front, aircraft_up);

    // Draw the scene
    draw_wait_time += dt_loop;
    if (callback_world.IsFPSLocked()) {
      if (draw_wait_time > 0.01666) {
        draw_wait_time = 0.0;
        DrawScene(terrain, sky, aircraft);
      }
    }
    else {
      DrawScene(terrain, sky, aircraft);
    }

    // Sleep (if possible)
    GLfloat end_loop_time = glfwGetTime();
    std::chrono::duration<float> sleep_duration(loop_lock_time - 
        (end_loop_time - current_loop_time));
    if (sleep_duration > std::chrono::duration<float>(0)) {
      std::this_thread::sleep_for(sleep_duration);
    }
   
    t_physics += dt_physics;
  } // End game loop

  GLEnvironment::TearDown();
  return 0;
}

void DrawScene(Terrain& terrain, Sky& sky, Aircraft& aircraft) {
  // Compute frame time
  GLfloat current_draw_time = glfwGetTime();
  GLfloat dt_draw = current_draw_time - last_draw_time;
  last_draw_time = current_draw_time;
  
  // Clear the colorbuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
  terrain.Draw(camera, sky);
  sky.Draw(camera);
  aircraft.Draw(camera, sky);
  // Display the debug console last
  debug_overlay.Draw(camera, dt_loop, dt_draw);
      
  // Swap the buffers
  glfwSwapBuffers(window);
}
