#include <iostream>
#include <cstdlib>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0";
const GLchar* orangeFragmentShaderSource = "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";
const GLchar* yellowFragmentShaderSource = "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
    "}\n\0";

// The MAIN function, from here we start the application and run the game loop
int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cout << "Run with './hello_triangle [problem#]'" << std::endl;
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
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // Check for compile time errors
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // Fragment shaders
  GLuint orangeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(orangeFragmentShader, 1, &orangeFragmentShaderSource, NULL);
  glCompileShader(orangeFragmentShader);
  GLuint yellowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(yellowFragmentShader, 1, &yellowFragmentShaderSource, NULL);
  glCompileShader(yellowFragmentShader);
  // Check for compile time errors
  glGetShaderiv(orangeFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(orangeFragmentShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  glGetShaderiv(yellowFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(yellowFragmentShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // Link shaders
  GLuint orangeShaderProgram = glCreateProgram();
  glAttachShader(orangeShaderProgram, vertexShader);
  glAttachShader(orangeShaderProgram, orangeFragmentShader);
  glLinkProgram(orangeShaderProgram);
  
  GLuint yellowShaderProgram = glCreateProgram();
  glAttachShader(yellowShaderProgram, vertexShader);
  glAttachShader(yellowShaderProgram, yellowFragmentShader);
  glLinkProgram(yellowShaderProgram);
  // Check for linking errors
  glGetProgramiv(orangeShaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
      glGetProgramInfoLog(orangeShaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  
  glGetProgramiv(yellowShaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
      glGetProgramInfoLog(yellowShaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(orangeFragmentShader);
  glDeleteShader(yellowFragmentShader);
  
  // Uncommenting this call will result in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  switch (prob_no) {
    case 1: {
      //-> Part 1: using one VAO to draw two triangles

      // Set up vertex data (and buffer(s)) and attribute pointers
      GLfloat vertices[] = {
        // First triangle
         0.5f,  0.5f, 0.0f, // Top Right
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f,  0.5f, 0.0f, // Top Left 
        // Second triangle
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f  // Top Left
      }; 
      
      GLuint VBO, VAO;
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      // Bind the Vertex Array Object first
      glBindVertexArray(VAO);
      // Copy vertex array into a vertex buffer to OpenGL to use
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      // Set the vertex attributes pointers
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        3*sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // Unbind the VAO
      glBindVertexArray(0);
  
      // Game loop
      while (!glfwWindowShouldClose(window))
      {
        // Check if any events have been activiated
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw our triangles
        glUseProgram(orangeShaderProgram);
        glBindVertexArray(VAO);
        // Draw 6 vertices, 3 for each triangle
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
      }
      // Properly de-allocate all resources once they've outlived their purpose
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);

      break;
    }
    case 2: {
      //-> Part 2: using two VAOs/VBOs to draw two triangles

      // Set up vertex data (and buffer(s)) and attribute pointers
      GLfloat vertices1[] = {
        // First triangle
         0.5f,  0.5f, 0.0f, // Top Right
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f,  0.5f, 0.0f  // Top Left 
      };
      GLfloat vertices2[] = {
        // Second triangle
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f  // Top Left
      }; 
      
      // Set up the first triangle
      GLuint VBO1, VAO1;
      glGenVertexArrays(1, &VAO1);
      glGenBuffers(1, &VBO1);
      // Bind the Vertex Array Object first
      glBindVertexArray(VAO1);
      // Copy vertex array into a vertex buffer to OpenGL to use
      glBindBuffer(GL_ARRAY_BUFFER, VBO1);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
      // Set the vertex attributes pointers
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        3*sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // Unbind the VAO
      glBindVertexArray(0);
      
      // Set up the second triangle
      GLuint VBO2, VAO2;
      glGenVertexArrays(1, &VAO2);
      glGenBuffers(1, &VBO2);
      // Bind the Vertex Array Object first
      glBindVertexArray(VAO2);
      // Copy vertex array into a vertex buffer to OpenGL to use
      glBindBuffer(GL_ARRAY_BUFFER, VBO2);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
      // Set the vertex attributes pointers
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        3*sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // Unbind the VAO
      glBindVertexArray(0);
  
      // Game loop
      while (!glfwWindowShouldClose(window))
      {
        // Check if any events have been activiated
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(orangeShaderProgram);

        // Draw the first triangle
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // Draw the second triangle
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // Unbind the VAO at the end
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
      }
      // Properly de-allocate all resources once they've outlived their purpose
      glDeleteVertexArrays(1, &VAO1);
      glDeleteBuffers(1, &VBO1);
      glDeleteVertexArrays(1, &VAO2);
      glDeleteBuffers(1, &VBO2);

      break;
    }
    case 3: {
      //-> Part 3: draw two triangles with different colors
      
      // Set up vertex data (and buffer(s)) and attribute pointers
      GLfloat vertices1[] = {
        // First triangle
         0.5f,  0.5f, 0.0f, // Top Right
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f,  0.5f, 0.0f  // Top Left 
      };
      GLfloat vertices2[] = {
        // Second triangle
         0.5f, -0.5f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f  // Top Left
      }; 
      
      // Set up the first triangle
      GLuint VBO1, VAO1;
      glGenVertexArrays(1, &VAO1);
      glGenBuffers(1, &VBO1);
      // Bind the Vertex Array Object first
      glBindVertexArray(VAO1);
      // Copy vertex array into a vertex buffer to OpenGL to use
      glBindBuffer(GL_ARRAY_BUFFER, VBO1);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
      // Set the vertex attributes pointers
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        3*sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // Unbind the VAO
      glBindVertexArray(0);
      
      // Set up the second triangle
      GLuint VBO2, VAO2;
      glGenVertexArrays(1, &VAO2);
      glGenBuffers(1, &VBO2);
      // Bind the Vertex Array Object first
      glBindVertexArray(VAO2);
      // Copy vertex array into a vertex buffer to OpenGL to use
      glBindBuffer(GL_ARRAY_BUFFER, VBO2);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
      // Set the vertex attributes pointers
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
        3*sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);
      // Unbind the VAO
      glBindVertexArray(0);
  
      // Game loop
      while (!glfwWindowShouldClose(window))
      {
        // Check if any events have been activiated
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the first triangle
        glUseProgram(orangeShaderProgram);
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // Draw the second triangle
        glUseProgram(yellowShaderProgram);
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // Unbind the VAO at the end
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
      }
      // Properly de-allocate all resources once they've outlived their purpose
      glDeleteVertexArrays(1, &VAO1);
      glDeleteBuffers(1, &VBO1);
      glDeleteVertexArrays(1, &VAO2);
      glDeleteBuffers(1, &VBO2);
      
      break;
    }
    default: {
      //-> Draw to triangles using an EBO
  
      GLfloat vertices[] = {
           0.5f,  0.5f, 0.0f,  // Top Right
           0.5f, -0.5f, 0.0f,  // Bottom Right
          -0.5f, -0.5f, 0.0f,  // Bottom Left
          -0.5f,  0.5f, 0.0f   // Top Left 
      };
      GLuint indices[] = {  // Note that we start from 0!
          0, 1, 3,  // First Triangle
          1, 2, 3   // Second Triangle
      };
      GLuint VBO, VAO, EBO;
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);
      // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);

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

        // Draw our first triangle
        glUseProgram(orangeShaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
      }
      // Properly de-allocate all resources once they've outlived their purpose
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);

      break;
    }
  }
  
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


