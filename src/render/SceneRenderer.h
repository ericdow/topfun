#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "terrain/Terrain.h"
#include "terrain/Sky.h"
#include "aircraft/Aircraft.h"
#include "render/Camera.h"
#include "shaders/Shader.h"

namespace TopFun {

class DepthMapRenderer;

inline void DrawScene(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
    const Camera& camera, const DepthMapRenderer& depthmap_renderer, 
    const Shader* shader=NULL) {
  terrain.Draw(camera, sky, depthmap_renderer, shader);
  sky.Draw(camera);
  // Always draw the aircraft last for canopy
  aircraft.Draw(camera, sky, depthmap_renderer, shader);
}

} // End namespace TopFun

#endif
