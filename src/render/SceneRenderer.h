#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "terrain/Terrain.h"
#include "terrain/Sky.h"
#include "aircraft/Aircraft.h"
#include "utils/Camera.h"
#include "shaders/Shader.h"

namespace TopFun {

inline void DrawScene(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
    const Camera& camera, const Shader* shader=NULL) {
  terrain.Draw(camera, sky, shader);
  sky.Draw(camera);
  // Always draw the aircraft last for canopy
  aircraft.Draw(camera, sky, shader);
}

} // End namespace TopFun

#endif
