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
  DebugOverlay(std::array<GLuint,2> const& screen_size) : visible_(false),
    text_renderer_(screen_size) {}

  ~DebugOverlay() = default;

  DebugOverlay(DebugOverlay const&) = delete;

  inline void ToggleVisible() {
    visible_ = !visible_;
  }

  void Draw(Camera const& camera, GLfloat delta_time) {
    if (visible_) {
      glm::vec3 text_color = glm::vec3(1.0, 1.0, 1.0);
      GLfloat scale = 0.4;
      GLfloat xt = 10;
      GLfloat yt = 10;
      GLfloat dyt = 50*scale;
      // Display the time it takes between each frame render 
      std::string s = "s/frame: " + std::to_string(delta_time);
      text_renderer_.Draw(s, xt, yt, scale, text_color);
      yt += dyt;

      // Display camera info
      glm::vec3 pos = camera.GetPosition();
      std::ostringstream x, y, z;
      x << std::setprecision(1) << std::fixed << pos.x;
      y << std::setprecision(1) << std::fixed << pos.y;
      z << std::setprecision(1) << std::fixed << pos.z;
      s = "Camera (x,y,z): (" + x.str() + "," + y.str() + "," + z.str() + ")";
      text_renderer_.Draw(s, xt, yt, scale, text_color);
      yt += dyt;
      
      glm::vec2 ang = camera.GetEulerAngles();
      std::ostringstream phi, theta;
      phi << std::setprecision(1) << std::fixed << ang.x;
      theta << std::setprecision(1) << std::fixed << ang.y;
      s = "Camera (a,b): (" + phi.str() + "," + theta.str() + ")";
      text_renderer_.Draw(s, xt, yt, scale, text_color);
      yt += dyt;
    }
  }

 private:
  bool visible_;
  TextRenderer text_renderer_;
  
};
} // End namespace TopFun

#endif