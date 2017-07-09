#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <vector>
#include <array>
#include <iomanip>

#include "utils/Camera.h"
#include "render/TextRenderer.h"
#include "aircraft/Aircraft.h"

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

  void Draw(const Camera& camera, const Aircraft& aircraft, 
      GLfloat delta_loop_time, GLfloat delta_draw_time) {
    if (visible_) {
      glm::vec3 text_color = glm::vec3(0.0, 0.0, 0.0);
      GLfloat scale = 0.5; // controls font size
      GLfloat xt = 10;
      GLfloat yt = 10;
      GLfloat dyt = 50*scale;

      std::vector<std::string> debug_strings;
      // Display the time it takes between each main loop iteration
      debug_strings.push_back("s/loop: " + std::to_string(delta_loop_time));
      
      // Display the FPS
      std::ostringstream fps;
      fps << std::setprecision(0) << std::fixed << std::round(1.0f/delta_draw_time);
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

      // Display the aircraft info
      std::ostringstream thr;
      thr << std::setprecision(1) << std::fixed << 
        (aircraft.GetThrottlePosition() * 100.0f);
      debug_strings.push_back("Throttle: " + thr.str() + "%");
      
      std::ostringstream vel;
      vel << std::setprecision(1) << std::fixed << 
        glm::l2Norm(aircraft.GetVelocity());
      debug_strings.push_back("Speed: " + vel.str() + " m/s");
      
      std::ostringstream alpha;
      alpha << std::setprecision(1) << std::fixed << 
        aircraft.GetAlpha() * 180.0f / M_PI;
      debug_strings.push_back("alpha: " + alpha.str());

      for (auto const& s : debug_strings) {
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
