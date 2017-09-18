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

// Sun parameters
uniform vec3 sun_dir;

// Define some constants
const float one_over_four_pi = 1.0 / 4.0 / 3.14159265;
const float g = 0.99;
const float k_schlick = 1.5 * g - 0.55 * g * g * g;
const float num_schlick = (1 - k_schlick*k_schlick) * one_over_four_pi;
const float sigma_extinction = 0.05; 
const float sigma_scattering = 0.7 * sigma_extinction;

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
  return max(0.0, da * (da - h) * one_over_h * one_over_h * -4.0f);
}

// Compute the cloud density at a particular xyz position
float GetDensity(vec3 position) {
  vec4 w = texture(weather, position.xz * weather_scale);
  float density = w.r;
  // density *= GetHeightSignal(w.g, w.b, position.y);
  vec4 shape_rgba = texture(shape, position * shape_scale);
  density *= 0.5*shape_rgba.r*(shape_rgba.g + shape_rgba.b + shape_rgba.a);
  vec4 density_rgba = texture(detail, position * detail_scale);
  density -= 0.1 * (density_rgba.r + density_rgba.g + density_rgba.b);
  // TODO height gradient...
  return clamp(density, 0.0, 1.0);
}

// Compute phase function, given normalized vectors
float CalcSunPhaseFunction(vec3 ray_dir) {
  float one_plus_k_cos_theta = 1.0 + k_schlick * dot(ray_dir, sun_dir);
  return num_schlick / (one_plus_k_cos_theta * one_plus_k_cos_theta);
}

// Exponential integral
float Ei(float z) {
  return 0.57722 + log(1e-4 + abs(z)) + 
    z * (1.0 + z * (0.25 + z * (0.055555 + z * (0.010416 + z * 0.0016666))));
}

vec3 CalcAmbientColor(vec3 position, float extinction_coeff) {
  float dtop = cloud_end - position.y;
  float a = -extinction_coeff * dtop;
  vec3 scattering_top = vec3(1.0, 1.0, 1.0) * max(0.0, exp(a) - a*Ei(a));
  float dbot = position.y - cloud_start;
  a = -extinction_coeff * dtop;
  vec3 scattering_bot = vec3(1.0, 1.0, 1.0) * max(0.0, exp(a) - a*Ei(a));
  return scattering_bot + scattering_top;
}

// Perform ray-marching to compute color and extinction
vec4 RayMarch(Ray ray, vec2 start_stop) {
  float extinction = 1.0;
  vec3 scattering = vec3(0.0, 0.0, 0.0);
  
  int n_steps = 10;
  float step_size = (start_stop.y - start_stop.x) / n_steps;
  vec3 position = ray.origin;
  vec3 dposition = step_size * ray.dir;
  for (int i = 0; i < n_steps; ++i) {
    float density = GetDensity(position);
    float scattering_coeff = sigma_scattering * density;
    float extinction_coeff = sigma_extinction * density;
    extinction *= exp(-extinction_coeff * step_size);

    // Check for early exit due to low transmittance
    if (extinction < 0.01)
      break;
    
    vec3 sun_color = vec3(1.0, 1.0, 1.0); // TODO
    vec3 ambient_color = CalcAmbientColor(position, extinction);
    vec3 step_scattering = scattering_coeff * step_size * 
      (CalcSunPhaseFunction(ray.dir) * sun_color + 
       one_over_four_pi * ambient_color);
    scattering += extinction * step_scattering;
    
    position += dposition;
  }
  return vec4(scattering, extinction);
}

// Compute start/stop positions for ray marching through atmosphere
vec2 GetRayAtmosphereIntersection(Ray ray) {
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
  l_stop_march = min(l_stop_march, l_start_march + l_stop_max);
  return vec2(l_start_march, l_stop_march);
}

void main() {    
  // Compute ray passing through each fragment
  Ray ray = GetFragRay();
  
  // Check where this ray intersects the cloud layer
  vec2 start_stop = GetRayAtmosphereIntersection(ray);

  // Perform ray-marching
  vec4 scatter_extinction = vec4(0.0, 0.0, 0.0, 1.0);
  if (start_stop.x >= 0.0f)
    scatter_extinction = RayMarch(ray, start_stop);
  
  ////////////////////////////////////////////////////////////////////
  float l_start_march = start_stop.x;
  float l_stop_march = start_stop.y;
  
  // TODO remove...
  if (l_start_march >= 0.0f) {
    // color = vec4(abs(ray.dir), 1.0);
    vec3 c = ray.origin + l_start_march * ray.dir;
    // color = texture(detail, c);
    // vec4 w = texture(weather, c.xz * weather_scale);
    // float tmp = GetHeightSignal(w.g, w.b, c.y + 3.0);
    // color = vec4(vec3(tmp), 1.0);
    color = scatter_extinction;
    if (scatter_extinction.a > 0.99)
      color = vec4(0.0, 0.0, 1.0, 1.0);
  }
  else {
    float scene_depth = texture(depth_map, TexCoord).r;
    color = vec4(vec3(LinearizeDepth(scene_depth) / camera_far), 1.0);
  }
  ////////////////////////////////////////////////////////////////////
  
}