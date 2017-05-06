// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

namespace TopFun {
namespace GLEnvironment {

GLFWwindow* SetUp(GLuint screenWidth, GLuint screenHeight);

void TearDown();

} // End namespace GLEnvironment
} // End namespace TopFun
