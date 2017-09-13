#version 330 core

#include "raymarch.glsl"

in vec2 TexCoord;

out vec4 color;

uniform sampler2D depth_map; // depth map of scene
uniform float cloud_start;
uniform float cloud_end;

// Cloud texture data
uniform sampler3D detail; // cloud detail texture
uniform float detail_scale;
uniform sampler3D shape; // cloud shape texture
uniform float shape_scale;
uniform sampler2D weather; // weather texture
uniform float weather_scale;

////////////////////////////////////////////////////////////////////
// TODO remove...
float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * camera_near * camera_far) / 
      (camera_far + camera_near - z * (camera_far - camera_near));	
}
////////////////////////////////////////////////////////////////////

void main() {    
  // Compute ray passing through each fragment
  Ray ray = GetFragRay();
  
  // Check where this ray intersects the cloud layer
  float l_start_march = -1.0f; // only march ray if this is positive
  float l_stop_march;
  if (ray.origin.y < cloud_end && ray.dir.y > 0.0f ||
      ray.origin.y > cloud_start && ray.dir.y < 0.0f) {
    float scene_depth = texture(depth_map, TexCoord).r;
    float l_stop;
    if (ray.origin.y > cloud_start && ray.origin.y < cloud_end) {
      // Ray starts inside cloud layer 
      l_start_march = 0.0f;
      if (ray.dir.y > 0.0f) {
        // Ray is looking up
        l_stop = CalcRayPlaneIntersection(ray, cloud_end); 
      }
      else {
        // Ray is looking down
        l_stop = CalcRayPlaneIntersection(ray, cloud_start);
      }
      vec3 xyz_int = ray.origin + l_stop * ray.dir;
      float d_int = CalcDepth(xyz_int);
      if (d_int < scene_depth) {
        l_stop_march = l_stop;
      }
    }
    else {
      float l_start;
      if (ray.origin.y < cloud_start) {
        // Ray starts below cloud layer and points up
        l_start = CalcRayPlaneIntersection(ray, cloud_start);
        l_stop = CalcRayPlaneIntersection(ray, cloud_end);
      }
      else {
        // Ray starts above cloud layer and points down
        l_start = CalcRayPlaneIntersection(ray, cloud_end);
        l_stop = CalcRayPlaneIntersection(ray, cloud_start);
      }
      vec3 xyz_int = ray.origin + l_start * ray.dir;
      float d_int = CalcDepth(xyz_int);
      if (d_int < scene_depth) {
        l_start_march = l_start;
      }
      xyz_int = ray.origin + l_stop * ray.dir;
      d_int = CalcDepth(xyz_int);
      if (d_int < scene_depth) {
        l_stop_march = l_stop;
      }
    }
  }
  l_stop_march = min(l_stop_march, l_stop_max);
  
  ////////////////////////////////////////////////////////////////////
  // TODO remove...
  if (l_start_march > 0.0f) {
    color = vec4(abs(ray.dir), 1.0);
    
    vec3 c = abs(ray.origin + l_start_march * ray.dir) / weather_scale;
    // color = texture(detail, c);
    color = vec4(vec3(texture(weather, c.xz).r), 1.0);
  }
  else {
    float scene_depth = texture(depth_map, TexCoord).r;
    color = vec4(vec3(LinearizeDepth(scene_depth) / camera_far), 1.0);
  }
  ////////////////////////////////////////////////////////////////////
  
}
