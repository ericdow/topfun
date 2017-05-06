#include <iostream>

#include "utils/GLEnvironment.h"
#include "utils/CallBackWorld.h"
#include "utils/Camera.h"
#include "terrain/Terrain.h"


// TODO move
// GLfloat lastX = width/2, lastY = height/2;


int main(int argc, char* argv[]) {
  using namespace TopFun;

  // Set up objects that will be modified during input callbacks 
  GLuint width(800), height(600);
  Camera camera(width, height);
  CallBackWorld call_back_world(camera);

  // Setup the GL/GLFW environment
  GLFWwindow* window = GLEnvironment::SetUp(width, height, call_back_world);
  
  Terrain terrain;

  // Game loop
  GLfloat deltaTime = 0.0f;
  GLfloat lastFrame = 0.0f;
  while(!glfwWindowShouldClose(window)) {
    // Set frame time
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check and call events
    glfwPollEvents();
    camera.Move(call_back_world.GetKeyState(), deltaTime);

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    // Draw the scene 
    terrain.Draw(camera);

    // Swap the buffers
    glfwSwapBuffers(window);
  }

  GLEnvironment::TearDown();
  return 0;
}
