#ifndef CALLBACKWORLD_H
#define CALLBACKWORLD_H

#include <vector>
#include <array>

#include "utils/Camera.h"

// Manager for objectis that will be modified by input callbacks

namespace TopFun {
  
class CallBackWorld {
 public:
  CallBackWorld(Camera& camera);

  ~CallBackWorld() = default;

  void ProcessKeyPress(int key, int scancode, int action, int mods);

  void ProcessMouseMovement(double xpos, double ypos);

  inline std::vector<bool> const& GetKeyState() {
    return key_state_;
  }

 private:
  bool first_mouse_;
  std::array<double,2> last_mouse_pos_;
  std::vector<bool> key_state_;
  bool w_double_pressed_;
  float last_w_press_time_;
  
  Camera& camera_;
  
};
} // End namespace TopFun

#endif
