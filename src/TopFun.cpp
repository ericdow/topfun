#include <iostream>
#include <chrono>
#include <thread>

#include <boost/numeric/odeint.hpp>

#include "utils/GLEnvironment.h"
#include "input/CallBackWorld.h"
#include "utils/Camera.h"
#include "render/DebugOverlay.h"
#include "terrain/Terrain.h"
#include "terrain/Sky.h"
#include "aircraft/Aircraft.h"
#include "render/SceneRenderer.h"
#include "render/DepthMapRenderer.h"

using namespace TopFun;

///////////////////////////////////////////////////////////////////////////////
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
  if (quadVAO == 0) {
   float quadVertices[] = {
     // positions        // texture Coords
     -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
     -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
   };
   // setup plane VAO
   glGenVertexArrays(1, &quadVAO);
   glGenBuffers(1, &quadVBO);
   glBindVertexArray(quadVAO);
   glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, 
       GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
       (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}
///////////////////////////////////////////////////////////////////////////////

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {1400, 800};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

// Set up objects that can be modified by input callbacks
GLfloat terrain_size = 100.0f; // 10000.0f;
glm::vec3 start_pos(terrain_size/2, 20.0f, terrain_size/2);
glm::vec3 scene_center(terrain_size/2, 0.0f, terrain_size/2);
Camera camera(screen_size, start_pos);
DebugOverlay debug_overlay(screen_size);
Aircraft aircraft(glm::vec3(terrain_size/2, 5.0f, terrain_size/2), 
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

  // Set up data for rendering shadows
  DepthMapRenderer depthmap_renderer(screen_size[0], screen_size[1]);

  // Point callback to correct location  
  GLEnvironment::SetCallback(window, callback_world);

  // Set up ODE integrator
  boost::numeric::odeint::runge_kutta4<std::vector<float>> integrator;
  
  // Game loop
  GLfloat last_loop_time = glfwGetTime();
  GLfloat draw_wait_time = 0.0f;
  float t_physics = 0.0f;
  const float dt_physics = 0.005f; // don't make this too big or small
  float t_accumulator = 0.0f;
  std::vector<float> current_state = aircraft.GetState();
  std::vector<float> previous_state = current_state;
  while(!glfwWindowShouldClose(window)) {
    // Compute loop time
    const GLfloat current_loop_time = glfwGetTime();
    dt_loop = current_loop_time - last_loop_time;
    last_loop_time = current_loop_time;
    t_accumulator += std::min(dt_loop, 0.25f);

    // Check and call events
    glfwPollEvents();

    // Update the aircraft state
    aircraft.UpdateControls(callback_world.GetKeyState());
    /*
    // integrator.do_step(boost::ref(aircraft), current_state, t_physics, 
    //     dt_loop);    
    // aircraft.SetState(current_state);
    while (t_accumulator >= dt_physics) {
      previous_state = current_state;
      integrator.do_step(boost::ref(aircraft), current_state, t_physics, 
          dt_physics);
      aircraft.SetState(current_state);
      t_physics += dt_physics;
      t_accumulator -= dt_physics;
    }
    const float alpha = t_accumulator / dt_physics;
    aircraft.InterpolateState(previous_state, current_state, alpha);
    aircraft.SetState(current_state);
    */
    
    // Update the camera position
    camera.Move(callback_world.GetKeyState(), dt_loop);
    // glm::vec3 aircraft_front = aircraft.GetFrontDirection();
    // glm::vec3 aircraft_up = aircraft.GetUpDirection();
    // camera.SetPosition(aircraft.GetPosition() + 
    //     2.0f * aircraft_up - 20.0f * aircraft_front);
    // camera.SetOrientation(aircraft_front, aircraft_up);

    if (true) {
    // if (false) {

    // Draw the scene
    draw_wait_time += dt_loop;
    if ((callback_world.IsFPSLocked() && draw_wait_time > 0.01666) || 
        !callback_world.IsFPSLocked()) {
      draw_wait_time = 0.0;
      // Compute frame time
      GLfloat current_draw_time = glfwGetTime();
      GLfloat dt_draw = current_draw_time - last_draw_time;
      last_draw_time = current_draw_time;

      // Render the depth map for drawing shadows
      depthmap_renderer.Render(terrain, sky, aircraft, camera,
         // -sky.GetSunDirection(), scene_center);
         scene_center - 100.0f * sky.GetSunDirection(), scene_center);
      
      // Clear the colorbuffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Render the scene
      DrawScene(terrain, sky, aircraft, camera, depthmap_renderer);
  
      // Display the debug console last
      debug_overlay.Draw(camera, aircraft, dt_loop, dt_draw);
  
      // Swap the buffers
      glfwSwapBuffers(window);
    }

    } else {
  
    //////////////////////////////////////////////////////////////////////////////
    // TODO remove
    Shader debug_shader("shaders/debug_quad.vs", "shaders/debug_quad.fs");
    depthmap_renderer.Render(terrain, sky, aircraft, camera,
       scene_center - 100.0f*sky.GetSunDirection(), scene_center); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  	debug_shader.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthmap_renderer.GetDepthMap());
    glUniform1i(glGetUniformLocation(debug_shader.GetProgram(), "depthMap"), 0);
    renderQuad();
    glfwSwapBuffers(window);
    //////////////////////////////////////////////////////////////////////////////
    
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
