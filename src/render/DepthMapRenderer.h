#ifndef DEPTHMAPRENDERER_H
#define DEPTHMAPRENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TopFun {

class Terrain;
class Sky;
class Aircraft;
class Shader;
class Camera;

class DepthMapRenderer {
 public:
  DepthMapRenderer(GLuint map_width, GLuint map_height);

  ~DepthMapRenderer() = default;

  inline GLuint GetDepthMap() const { return depth_map_; }

  void Render(Terrain& terrain, Sky& sky, Aircraft& aircraft, 
      const Camera& camera, const glm::mat4& proj_view, 
      const Shader& shader);

 private:
  GLuint map_width_;
  GLuint map_height_;
  GLuint depth_map_; // texture storing depth map
  GLuint depth_mapFBO_;

};
} // End namespace TopFun

#endif
