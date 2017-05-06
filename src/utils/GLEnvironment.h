// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

namespace TopFun {

class CallBackWorld;

namespace GLEnvironment {

GLFWwindow* SetUp(GLuint screen_width, GLuint screen_height, 
    CallBackWorld& call_back_world);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, 
    int mods);

void MouseCallback(GLFWwindow* window, double xpos, double ypos);

void TearDown();

} // End namespace GLEnvironment
} // End namespace TopFun
