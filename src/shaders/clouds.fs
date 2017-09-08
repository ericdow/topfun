#version 330 core

#include "raymarch.glsl"

in vec2 TexCoord;

out vec4 color;

uniform sampler2D depth_map; // depth map of scene
uniform float cloud_start;
uniform float cloud_end;

// TODO remove...
float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * camera_near * camera_far) / 
      (camera_far + camera_near - z * (camera_far - camera_near));	
}

void main() {    
  // ray-direction test...
  Ray ray = GetFragRay();
  color = vec4(abs(ray.dir), 1.0);
  
  // depth test... 
  float scene_depth = texture(depth_map, TexCoord).r;
  color = vec4(vec3(LinearizeDepth(scene_depth) / camera_far), 1.0);

  // Determine if this ray intersects the cloud layer
  if (ray.origin.y < cloud_end && ray.dir.y > 0 ||
      ray.origin.y > cloud_start && ray.dir.y < 0) {
    color = vec4(1.0, 0.0, 0.0, 1.0);
  }
}
