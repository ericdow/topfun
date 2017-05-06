#include <iostream>

#include "utils/GLEnvironment.h"
#include "utils/Camera.h"
#include "utils/KeyState.h"
#include "terrain/Terrain.h"

// TODO move
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
GLuint width(800), height(600);
GLFWwindow* window = TopFun::GLEnvironment::SetUp(width, height);
TopFun::KeyState key_state(window);
Camera camera(width, height);
GLfloat lastX = width/2, lastY = height/2;
bool firstMouse = true;

int main(int argc, char* argv[]) {
  using namespace TopFun;

  // TODO move
  glfwSetCursorPosCallback(window, mouse_callback);
  
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
    camera.Move(key_state.Get(), deltaTime);

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

// TODO move
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  if(firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset = lastY - ypos; 
  
  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}	
