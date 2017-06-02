#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <vector>
#include <array>
#include <iomanip>

#include "utils/Camera.h"
#include "utils/TextRenderer.h"

// Prints debug/performance info to the screen

namespace TopFun {
  
class DebugOverlay {
 public:
  DebugOverlay(std::array<GLuint,2> const& screen_size) : visible_(true),
    text_renderer_(screen_size) {}

  ~DebugOverlay() = default;

  DebugOverlay(DebugOverlay const&) = delete;

  inline void ToggleVisible() {
    visible_ = !visible_;
  }

  void Draw(Camera const& camera, GLfloat delta_loop_time, 
      GLfloat delta_draw_time) {
    if (visible_) {
      glm::vec3 text_color = glm::vec3(1.0, 1.0, 1.0);
      GLfloat scale = 0.45;
      GLfloat xt = 10;
      GLfloat yt = 10;
      GLfloat dyt = 50*scale;

      std::vector<std::string> debug_strings;
      // Display the time it takes between each main loop iteration
      debug_strings.push_back("s/loop: " + std::to_string(delta_loop_time));
      
      // Display the FPS
      std::ostringstream fps;
      fps << std::setprecision(1) << std::fixed << 1.0/delta_draw_time;
      debug_strings.push_back("frames/s: " + fps.str());

      // Display camera info
      glm::vec3 pos = camera.GetPosition();
      std::ostringstream x, y, z;
      x << std::setprecision(1) << std::fixed << pos.x;
      y << std::setprecision(1) << std::fixed << pos.y;
      z << std::setprecision(1) << std::fixed << pos.z;
      debug_strings.push_back("Camera (x,y,z): (" + x.str() + "," + 
          y.str() + "," + z.str() + ")");
      
      glm::vec3 ang = camera.GetEulerAngles();
      std::ostringstream phi, theta, psi;
      phi << std::setprecision(1) << std::fixed << ang.x;
      theta << std::setprecision(1) << std::fixed << ang.y;
      psi << std::setprecision(1) << std::fixed << ang.z;
      debug_strings.push_back("Camera (a,b): (" + phi.str() + "," + 
          theta.str() + "," + psi.str() + ")");

      for (auto& s : debug_strings) {
        text_renderer_.Draw(s, xt, yt, scale, text_color);
        yt += dyt;
      }
    }
  }

 private:
  bool visible_;
  TextRenderer text_renderer_;
  
};
} // End namespace TopFun

#endif
