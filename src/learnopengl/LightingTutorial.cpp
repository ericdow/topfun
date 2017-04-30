#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "SOIL.h"
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"

// Initialize camera defintion (yay global variables...)
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f); // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Target position
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f); // Up vector

// bool array of which keys are currently being pushed
bool keys[1024];

GLfloat deltaTime = 0.0f; // Time between current frame and last frame
GLfloat lastFrame = 0.0f;   // Time of last frame

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Starting position for the mouse cursor
GLfloat lastX = WIDTH/2, lastY = HEIGHT/2;
bool firstMouse = true;
GLfloat yaw = -90.0f, pitch = 0.0f;

// The MAIN function, from here we start the application and run the game loop
int main()
{
  // Init GLFW
  glfwInit();
  
  // Set all the required options for GLFW
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a GLFWwindow object that we can use for GLFW's functions
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // Set the required callback functions
  glfwSetKeyCallback(window, key_callback);

  // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
  glewExperimental = GL_TRUE;
  // Initialize GLEW to setup the OpenGL Function pointers
  glewInit();

  // Define the viewport dimensions
  glViewport(0, 0, WIDTH, HEIGHT);

  // Build and compile our shader programs
  Shader boxShader("box.vs", "box.frag");
  Shader lampShader("lamp.vs", "lamp.frag");

  // Set up vertex data (and buffer(s)) and attribute pointers
  GLfloat vertices[] = {
      // Positions
  -0.5f, -0.5f, -0.5f, 
   0.5f, -0.5f, -0.5f, 
   0.5f,  0.5f, -0.5f, 
   0.5f,  0.5f, -0.5f, 
  -0.5f,  0.5f, -0.5f, 
  -0.5f, -0.5f, -0.5f, 

  -0.5f, -0.5f,  0.5f, 
   0.5f, -0.5f,  0.5f, 
   0.5f,  0.5f,  0.5f, 
   0.5f,  0.5f,  0.5f, 
  -0.5f,  0.5f,  0.5f, 
  -0.5f, -0.5f,  0.5f, 

  -0.5f,  0.5f,  0.5f, 
  -0.5f,  0.5f, -0.5f, 
  -0.5f, -0.5f, -0.5f, 
  -0.5f, -0.5f, -0.5f, 
  -0.5f, -0.5f,  0.5f, 
  -0.5f,  0.5f,  0.5f, 

   0.5f,  0.5f,  0.5f, 
   0.5f,  0.5f, -0.5f, 
   0.5f, -0.5f, -0.5f, 
   0.5f, -0.5f, -0.5f, 
   0.5f, -0.5f,  0.5f, 
   0.5f,  0.5f,  0.5f, 

  -0.5f, -0.5f, -0.5f, 
   0.5f, -0.5f, -0.5f, 
   0.5f, -0.5f,  0.5f, 
   0.5f, -0.5f,  0.5f, 
  -0.5f, -0.5f,  0.5f, 
  -0.5f, -0.5f, -0.5f, 

  -0.5f,  0.5f, -0.5f, 
   0.5f,  0.5f, -0.5f, 
   0.5f,  0.5f,  0.5f, 
   0.5f,  0.5f,  0.5f, 
  -0.5f,  0.5f,  0.5f, 
  -0.5f,  0.5f, -0.5f 
  };
 
  // Set up a VBO with box coordinate data 
  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set up VAO for boxes
  GLuint boxVAO;
  glGenVertexArrays(1, &boxVAO);
  glBindVertexArray(boxVAO);
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0); // Unbind VAO
  
  // Specify positions to render multiple cubes
  glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
  };

  // Set up VAO for lamp
	GLuint lampVAO;
  glGenVertexArrays(1, &lampVAO);
  glBindVertexArray(lampVAO);
  // We only need to bind to the VBO, since the VBO data already contains the correct data.
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // Set the vertex attributes (only position data for our lamp)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0); 

  // Make sure z-buffering is enabled
  glEnable(GL_DEPTH_TEST);

  // Make sure cursor is hidden and stays in window
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

  // Set callback function for mouse movement
  glfwSetCursorPosCallback(window, mouse_callback);

  // Game loop
  while (!glfwWindowShouldClose(window))
  {
    // Compute dt from last render
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
    glfwPollEvents();
    do_movement();

    // Render
    // Clear the colorbuffer and depthbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // View transformation moves the camera to its location in the world
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Projection transformation defines the clipping frustrum
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(60.0f), 
      (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

    // Get transformations uniform location and set matrices for drawing boxes
    GLint modelLoc_box = glGetUniformLocation(boxShader.Program, "model");
    GLint viewLoc_box = glGetUniformLocation(boxShader.Program, "view");
    GLint projLoc_box = glGetUniformLocation(boxShader.Program, "projection");
    glUniformMatrix4fv(viewLoc_box, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc_box, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Activate shader for drawing boxes and set uniforms
    boxShader.Use();       
    GLint objectColorLoc = glGetUniformLocation(boxShader.Program, "objectColor");
    GLint lightColorLoc  = glGetUniformLocation(boxShader.Program, "lightColor");
    glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
    glUniform3f(lightColorLoc,  1.0f, 1.0f, 1.0f); // Also set light's color (white) 
    
    // Draw boxes
    glBindVertexArray(boxVAO);
    for(GLuint i = 0; i < 10; i++)
    {
      // Model transformation moves the object model to its location/orientation in the world
      glm::mat4 model;
      model = glm::translate(model, cubePositions[i]);
      GLfloat angle = 1.0f * i * (GLfloat)glfwGetTime();
      model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
      glUniformMatrix4fv(modelLoc_box, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    
    // Get transformations uniform location and set matrices for drawing a lamp
    GLint modelLoc_lamp = glGetUniformLocation(lampShader.Program, "model");
    GLint viewLoc_lamp = glGetUniformLocation(lampShader.Program, "view");
    GLint projLoc_lamp = glGetUniformLocation(lampShader.Program, "projection");
    glUniformMatrix4fv(viewLoc_lamp, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc_lamp, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Activate shader for drawing lamp and set uniforms
    lampShader.Use();
    
    // Draw lamp
    glBindVertexArray(lampVAO);
    // Model transformation moves the object model to its location/orientation in the world
    glm::vec3 lampPos(1.2f, 1.0f, 2.0f);
    glm::mat4 model;
    model = glm::translate(model, lampPos);
    model = glm::scale(model, glm::vec3(0.2f));
    glUniformMatrix4fv(modelLoc_lamp, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }
  // Properly de-allocate all resources once they've outlived their purpose
  glDeleteVertexArrays(1, &boxVAO);
  glDeleteVertexArrays(1, &lampVAO);
  glDeleteBuffers(1, &VBO);
  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwTerminate();
  return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  // Close by pressing escape key
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
  // Update the state machine of which keys are pressed
  if(action == GLFW_PRESS)
    keys[key] = true;
  else if(action == GLFW_RELEASE)
    keys[key] = false;  
}

// Update the camera position based on key presses
void do_movement() {
    GLfloat cameraSpeed = 10.0f * deltaTime; // Ensure uniform movement speed
    if(keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if(keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if(keys[GLFW_KEY_A])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(keys[GLFW_KEY_D])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Update the camera target based on mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  // Check if this is the time the mouse enters the screen to prevent jumps
  if(firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset = lastY - ypos; 
  lastX = xpos;
  lastY = ypos;

  GLfloat sensitivity = 0.2f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw = glm::mod(yaw + xoffset, 360.0f);
  pitch += yoffset;

  if(pitch > 89.0f)
      pitch = 89.0f;
  if(pitch < -89.0f)
      pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}  
