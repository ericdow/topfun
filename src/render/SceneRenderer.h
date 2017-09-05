#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "terrain/Terrain.h"
#include "sky/Sky.h"
#include "aircraft/Aircraft.h"
#include "render/Camera.h"
#include "shaders/Shader.h"

namespace TopFun {

class ShadowCascadeRenderer;

inline void DrawScene(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
    const Camera& camera, const ShadowCascadeRenderer* pshadow_renderer,
    const Shader* shader=NULL) {
  terrain.Draw(camera, sky, pshadow_renderer, shader);
  // Only draw the sky if not rendering shadows
  if (!shader) {
    sky.Draw(camera);
  }
  // Always draw the aircraft last for canopy
  aircraft.Draw(camera, sky, pshadow_renderer, shader);
}

} // End namespace TopFun

#endif
