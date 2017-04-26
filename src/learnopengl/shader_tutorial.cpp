#include <iostream>
#include <cstdlib>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Vertex shaders
const GLchar* defaultVertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0";
const GLchar* rainbowVertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 color;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(position, 1.0f);\n"
    "ourColor = color;\n"
    "}\0";

// Fragment shaders    
const GLchar* defaultFragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";
const GLchar* sinusoidFragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 color;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "color = ourColor;\n"
    "}\n\0";
const GLchar* rainbowFragmentShaderSource = 
    "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(ourColor, 1.0f);\n"
    "}\n\0";

// The MAIN function, from here we start the application and run the game loop
int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cout << "Run with './shader_tutorial [problem#]'" << std::endl;
    throw;
  }

  int prob_no = atoi(argv[1]);

  std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
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
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);  
  glViewport(0, 0, width, height);

  // Build and compile our shader program
  // Vertex shader
  GLuint defaultVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(defaultVertexShader, 1, &defaultVertexShaderSource, NULL);
  glCompileShader(defaultVertexShader);
  
  GLuint rainbowVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(rainbowVertexShader, 1, &rainbowVertexShaderSource, NULL);
  glCompileShader(rainbowVertexShader);
  
  // Fragment shaders
  GLuint defaultFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(defaultFragmentShader, 1, &defaultFragmentShaderSource, NULL);
  glCompileShader(defaultFragmentShader);
  
  GLuint sinusoidFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(sinusoidFragmentShader, 1, &sinusoidFragmentShaderSource, NULL);
  glCompileShader(sinusoidFragmentShader);
  
  GLuint rainbowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(rainbowFragmentShader, 1, &rainbowFragmentShaderSource, NULL);
  glCompileShader(rainbowFragmentShader);
  
  // Link shaders
  GLuint defaultShaderProgram = glCreateProgram();
  glAttachShader(defaultShaderProgram, defaultVertexShader);
  glAttachShader(defaultShaderProgram, defaultFragmentShader);
  glLinkProgram(defaultShaderProgram);
  
  GLuint sinusoidShaderProgram = glCreateProgram();
  glAttachShader(sinusoidShaderProgram, defaultVertexShader);
  glAttachShader(sinusoidShaderProgram, sinusoidFragmentShader);
  glLinkProgram(sinusoidShaderProgram);
  
  GLuint rainbowShaderProgram = glCreateProgram();
  glAttachShader(rainbowShaderProgram, rainbowVertexShader);
  glAttachShader(rainbowShaderProgram, rainbowFragmentShader);
  glLinkProgram(rainbowShaderProgram);
  
  // Clean up shaders
  glDeleteShader(defaultVertexShader);
  glDeleteShader(rainbowVertexShader);

  glDeleteShader(defaultFragmentShader);
  glDeleteShader(sinusoidFragmentShader);
  glDeleteShader(rainbowFragmentShader);
  
  // Uncommenting this call will result in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
  //-> Draw a triangle using an EBO
  GLfloat vertices[] = {
    // Positions       
     0.5f, -0.5f, 0.0f, // Bottom Right
    -0.5f, -0.5f, 0.0f, // Bottom Left
     0.0f,  0.5f, 0.0f  // Top 
  };  
  GLfloat vertices2[] = {
    // Positions         // Colors
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // Bottom Right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // Bottom Left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // Top 
  };  
  GLuint indices[] = {  // Note that we start from 0!
      0, 1, 2  // First Triangle
  };
  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  switch (prob_no) {
    case 1:
    default: {
      //-> vertex data only contains position attribute
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 
        (GLvoid*)0);
      glEnableVertexAttribArray(0);
      break;
    }
    case 2: {
      //-> vertex data contains position and color attributes
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
      // position attribute
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 
        (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // color attribute
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 
        (GLvoid*)(3*sizeof(GLfloat)));
      glEnableVertexAttribArray(1);
      break;
    }
    case 3: {
      
      break;
    }
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

  glBindVertexArray(0); // Unbind VAO, remember: do NOT unbind the EBO, keep it bound to this VAO

  // Game loop
  while (!glfwWindowShouldClose(window))
  {
    // Check if any events have been activiated
    glfwPollEvents();

    // Render
    // Clear the colorbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw our triangle
    switch (prob_no) {
      case 1: {
        //-> specify a time-varying green value for this fragment shader
        GLfloat timeValue = glfwGetTime();
        GLfloat greenValue = (sin(timeValue) / 2) + 0.5;
        GLint vertexColorLocation = 
          glGetUniformLocation(sinusoidShaderProgram, "ourColor");
        glUseProgram(sinusoidShaderProgram);
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        break;
      }
      case 2: {
        //-> rainbow!!!
        glUseProgram(rainbowShaderProgram);
        break;
      }
      case 3: {
        
        break;
      }
      default: {
        
        break;
      }
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }
  // Properly de-allocate all resources once they've outlived their purpose
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwTerminate();
  return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}


