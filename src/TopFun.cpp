#include <iostream>
#include <chrono>
#include <thread>

#include "utils/GLEnvironment.h"
#include "input/CallBackWorld.h"
#include "render/Camera.h"
#include "render/DebugOverlay.h"
#include "terrain/Terrain.h"
#include "sky/Sky.h"
#include "sky/CloudRenderer.h"
#include "aircraft/Aircraft.h"
#include "render/SceneRenderer.h"
#include "render/ShadowCascadeRenderer.h"
#include "audio/AudioManager.h"

using namespace TopFun;

// Set up the GL/GLFW environment
const std::array<GLuint,2> screen_size = {{1400, 800}};
GLFWwindow* window = GLEnvironment::SetUp(screen_size);

// Set up objects that can be modified by input callbacks
GLfloat terrain_size = 150000.0f;
glm::vec3 start_pos(0.0, 10.0f, 0.0);
glm::vec3 scene_center(terrain_size/2, 0.0f, terrain_size/2);
Camera camera(screen_size, start_pos);
DebugOverlay debug_overlay(screen_size);
ShadowCascadeRenderer shadow_renderer(4*screen_size[0], 4*screen_size[1], 
    {0.0005, 0.0015, 0.005, 0.015, 0.05}, {0.002, 0.002, 0.003, 0.01, 0.1});
CallBackWorld callback_world(camera, debug_overlay, shadow_renderer, 
    screen_size);

GLfloat last_draw_time = 0.0f;
GLfloat dt_loop = 0.0f;
// Force loop to sleep until this amount of time has passed
GLfloat loop_lock_time = 1.0/120.0;
int main(int /* argc */, char** /* argv */) {
  // Setup the audio manager and load audio files
  AudioManager::SetUp();
  AudioManager::Instance().AddBuffer("../../../assets/audio/engine_idle.wav", 
      "engine_idle");
  AudioManager::Instance().AddBuffer("../../../assets/audio/afterburner.wav", 
      "afterburner");
  
  // Set up remaining game objects (in main due to static members)
  Terrain terrain(terrain_size, 19, {{start_pos[0], start_pos[2]}});
  Aircraft aircraft(start_pos,
      glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      camera, terrain);
  Sky sky;
  CloudRenderer cloud_renderer(screen_size[0], screen_size[1]);

  // Point callback to correct location  
  GLEnvironment::SetCallback(window, callback_world);
  
  // Game loop
  GLfloat last_loop_time = glfwGetTime();
  GLfloat draw_wait_time = 0.0f;
  float t_physics = 0.0f;
  const float dt_physics = 0.005f; // don't make this too big or small
  float t_accumulator = 0.0f;
  std::vector<double> current_state = aircraft.GetState();
  std::vector<double> previous_state = current_state;
  while(!glfwWindowShouldClose(window)) {
    // Compute loop time
    const GLfloat current_loop_time = glfwGetTime();
    dt_loop = current_loop_time - last_loop_time;
    last_loop_time = current_loop_time;
    t_accumulator += std::min(dt_loop, 0.25f);

    // Check and call events
    glfwPollEvents();

    // Update the aircraft state
    aircraft.SetState(current_state); // restore after rendering
    aircraft.UpdateControls(callback_world.GetKeyState());
    while (t_accumulator >= dt_physics) {
      previous_state = current_state;
      aircraft.DoPhysicsStep(t_physics, dt_physics);
      current_state = aircraft.GetState();
      t_physics += dt_physics;
      t_accumulator -= dt_physics;
    }
    // Interpolate the state vector for rendering
    const float alpha = t_accumulator / dt_physics;
    std::vector<double> render_state = aircraft.InterpolateState(previous_state,
        current_state, alpha);
    aircraft.SetState(render_state);
    
    // Update the camera position
    auto look_type = callback_world.GetLookType();
    if (look_type == LookType::free) {
      camera.Move(callback_world.GetKeyState(), dt_loop);
      AudioManager::Instance().SetListenerVelocity(glm::vec3(0.0, 0.0, 0.0));
    }
    else if (look_type == LookType::follow) {
      glm::vec3 aircraft_front = aircraft.GetFrontDirection();
      glm::vec3 aircraft_up = aircraft.GetUpDirection();
      camera.SetPosition(aircraft.GetPosition() + 
          aircraft.GetDeltaCenterOfMass() +
          2.0 * (glm::dvec3)aircraft_up - 20.0 * (glm::dvec3)aircraft_front);
      auto cam_pos = camera.GetPosition();
      float y_terrain = terrain.GetHeight(cam_pos[0], cam_pos[2]);
      if (cam_pos[1] < y_terrain + 1.0) {
        cam_pos[1] = y_terrain + 1.0;
        camera.SetPosition(cam_pos);
      }
      camera.SetOrientation(aircraft_front, aircraft_up);
      AudioManager::Instance().SetListenerVelocity(aircraft.GetVelocity());
    }
    else if (look_type == LookType::track) {
      glm::vec3 up(0.0f, 1.0f, 0.0f);
      glm::vec3 front = aircraft.GetPosition() + aircraft.GetDeltaCenterOfMass()
        - camera.GetPosition();
      camera.SetOrientation(front, up);
      AudioManager::Instance().SetListenerVelocity(glm::vec3(0.0, 0.0, 0.0));
    }
    AudioManager::Instance().SetListenerPosition(camera.GetPosition());
    AudioManager::Instance().SetListenerOrientation(camera.GetOrientation());

    // Update terrain tiles
    glm::vec3 camera_pos = camera.GetPosition();
    terrain.SetXZCenter({{camera_pos[0], camera_pos[2]}});

    // Draw the scene
    draw_wait_time += dt_loop;
    if ((callback_world.IsFPSLocked() && draw_wait_time > 0.01666) || 
        !callback_world.IsFPSLocked()) {
      draw_wait_time = 0.0;
      // Compute frame time
      GLfloat current_draw_time = glfwGetTime();
      GLfloat dt_draw = current_draw_time - last_draw_time;
      last_draw_time = current_draw_time;

      // Render the depth maps for drawing shadows
      shadow_renderer.Render(terrain, sky, aircraft, camera,
          -sky.GetSunDirection());

      // Render the clouds to a texture
      cloud_renderer.RenderToTexture(terrain, sky, aircraft, camera);
      
      // Clear the colorbuffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Render the scene
      DrawScene(terrain, sky, aircraft, camera, &shadow_renderer);

      // Blend the clouds with the scene
      cloud_renderer.BlendWithScene();

      // Display the depth map for debugging
      shadow_renderer.Display();
  
      // Display the debug console last
      debug_overlay.Draw(camera, aircraft, dt_loop, dt_draw);
      
      // Swap the buffers
      glfwSwapBuffers(window);
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
  AudioManager::TearDown();
  return 0;
}
