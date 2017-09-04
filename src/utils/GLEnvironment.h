#ifndef GLENVIRONMENT_H
#define GLENVIRONMENT_H

#include <string>
#include <array>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

// GLFW
#include <GLFW/glfw3.h>

namespace TopFun {

class CallBackWorld;

namespace GLEnvironment {

GLFWwindow* SetUp(std::array<GLuint,2> const& screen_size); 
 
void SetCallback(GLFWwindow* window, CallBackWorld& call_back_world);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, 
    int mods);

void MouseCallback(GLFWwindow* window, double xpos, double ypos);

glm::ivec4 GetViewport();

void TearDown();

} // End namespace GLEnvironment
} // End namespace TopFun

#endif
