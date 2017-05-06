#include "utils/GLEnvironment.h"
#include "terrain/Terrain.h"

// The MAIN function, from here we start our application and run our Game loop
int main(int argc, char* argv[]) {
  using namespace TopFun;

  GLFWwindow* window = GLEnvironment::SetUp(800, 600);

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

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    
    Terrain terrain;
    terrain.Draw();
  

    // Swap the buffers
    glfwSwapBuffers(window);
  }

  GLEnvironment::TearDown();
  return 0;
}
