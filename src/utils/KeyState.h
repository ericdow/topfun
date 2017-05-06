#ifndef KEYSTATE_H
#define KEYSTATE_H

#include <vector>
#include <GLFW/glfw3.h>

namespace TopFun {

class KeyState {
 
 public:
  KeyState(GLFWwindow* window);

  ~KeyState() = default;

  inline std::vector<bool> const& Get() {return keys;}

 private:
  static std::vector<bool> keys;

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
      int mode);

};
} // End namespace TopFun

#endif
