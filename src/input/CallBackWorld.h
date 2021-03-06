#ifndef CALLBACKWORLD_H
#define CALLBACKWORLD_H

#include <vector>
#include <array>

#include "render/Camera.h"
#include "render/DebugOverlay.h"

// Manager for objects that will be modified by input callbacks

namespace TopFun {

enum class LookType {
  follow,
  track,
  free,
};

class ShadowCascadeRenderer;

class CallBackWorld {
 public:
  CallBackWorld(Camera& camera, DebugOverlay& debug_overlay, 
     ShadowCascadeRenderer& shadow_renderer, 
     std::array<GLuint,2> const& screen_size);

  ~CallBackWorld() = default;

  void ProcessKeyPress(int key, int scancode, int action, int mods);

  void ProcessMouseMovement(double xpos, double ypos);

  inline std::vector<bool> const& GetKeyState() const {
    return key_state_;
  }
  
  inline bool IsFPSLocked() const {
    return fps_locked_;
  }
  
  inline LookType GetLookType() const {
    return look_type_;
  }

 private:
  bool first_mouse_;
  std::array<double,2> last_mouse_pos_;
  std::vector<bool> key_state_;
  bool fps_locked_;
  bool w_double_pressed_;
  float last_w_press_time_;
  LookType look_type_;
  
  Camera& camera_;
  DebugOverlay& debug_overlay_;
  ShadowCascadeRenderer& shadow_renderer_;

};
} // End namespace TopFun

#endif
