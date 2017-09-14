#version 330 core

#include "raymarch.glsl"

in vec2 TexCoord;

out vec4 color;

uniform sampler2D depth_map; // depth map of scene

// Cloud texture data
uniform sampler3D detail; // cloud detail texture
uniform float detail_scale;
uniform sampler3D shape; // cloud shape texture
uniform float shape_scale;
uniform sampler2D weather; // weather texture
uniform float weather_scale;

// Cloud density parameters
uniform float cloud_start;
uniform float cloud_end;
uniform float max_cloud_height; // maximum cloud vertical thickness

////////////////////////////////////////////////////////////////////
// TODO remove...
float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * camera_near * camera_far) / 
      (camera_far + camera_near - z * (camera_far - camera_near));	
}
////////////////////////////////////////////////////////////////////

// height - fraction in [0, 1] of maximum cloud height at this location
// altitude - fraction in [0, 1] of maximum cloud altitude
// y - y location of this point
float GetHeightSignal(float height, float altitude, float y) {
  float h = max_cloud_height * height;
  float one_over_h = 1.0f / h;
  float da = y - (cloud_start + 
    (cloud_end - cloud_start - max_cloud_height / 2.0) * altitude);
  return max(0, da * (da - h) * one_over_h * one_over_h * -4.0f);
}

// Compute the cloud density at a particular xyz position
float GetDensity(vec3 position) {
  vec4 w = texture(weather, position.xz * weather_scale);
  float density = w.r;
  density *= GetHeightSignal(w.g, w.b, position.y);
  density *= texture(shape, position * shape_scale);
  density -= texture(detail, position * detail_scale);
  // TODO height gradient...
  return density;
}

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
    
    vec3 c = ray.origin + l_start_march * ray.dir;
    // color = texture(detail, c);
    // color = vec4(vec3(texture(weather, c.xz).r), 1.0);
    vec4 w = texture(weather, c.xz * weather_scale);
    float tmp = GetHeightSignal(w.g, w.b, c.y + 3.0);
    color = vec4(vec3(tmp), 1.0);
  }
  else {
    float scene_depth = texture(depth_map, TexCoord).r;
    color = vec4(vec3(LinearizeDepth(scene_depth) / camera_far), 1.0);
  }
  ////////////////////////////////////////////////////////////////////
  
}
