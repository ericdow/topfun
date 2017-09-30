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
uniform sampler2D weather; // weather texture (coverage/height/altitude)
uniform float weather_scale;
uniform sampler2D texture_prev; // cloud texture from previous render

// Cloud density parameters
uniform float cloud_start;
uniform float cloud_end;
uniform float max_cloud_height; // maximum cloud vertical thickness

// Sun parameters
uniform vec3 sun_dir; // normalized, points towards sun
uniform vec3 sun_color;

// Define some constants for phase functions
const float one_over_four_pi = 1.0 / 4.0 / 3.14159265;
const float g = 0.9;
const float k_schlick = 1.5 * g - 0.55 * g * g * g;
const float num_schlick = (1 - k_schlick*k_schlick) * one_over_four_pi;
const float sigma_extinction = 1.0; // larger is thicker
const float sigma_scattering = 0.95; // larger is brighter

// Define some constants for shadows
const int n_shadow_steps = 4; // step_size0_over_l must be consistent
const float step_size_growth = 1.2;
const float step_size0_over_l = 1.0 / (1.0 + step_size_growth + 
  pow(step_size_growth, 2) + pow(step_size_growth, 3));

// Parabolic density multiplier
float GetHeightAttenuation(float h_frac) {
  return max(0.0, h_frac * (h_frac - 1.0) * -4.0f);
}

float GetHeightGradient(float h_frac) {
  float delta = 0.6;
  return clamp((1.0 + delta) * h_frac - delta, 0.0, 1.0);
}

// Compute the cloud start height based on the height and altitude values
// h - cloud height at this location
// altitude - fraction in [0, 1] of maximum cloud altitude
float GetCloudStartHeight(float h, float altitude) {
  return cloud_start + 0.5 * (max_cloud_height - h) + 
    (cloud_end - cloud_start - max_cloud_height) * altitude;
}

// Compute the cloud density at a particular xyz position
float GetDensity(vec3 position, vec4 w, float h_frac) {
  float density = w.r;
  density *= GetHeightAttenuation(h_frac);
  density *= texture(shape, position * shape_scale).r;
  density -= 0.1 * texture(detail, position * detail_scale).r;
  density *= GetHeightGradient(h_frac);
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

// Expensive ambient color function
vec3 CalcAmbientColor(vec3 position, float extinction_coeff) {
  float dtop = cloud_end - position.y;
  float a = -extinction_coeff * dtop;
  vec3 scattering_top = vec3(1.0, 1.0, 1.0) * max(0.0, exp(a) - a*Ei(a));
  float dbot = position.y - cloud_start;
  a = -extinction_coeff * dbot;
  vec3 scattering_bot = vec3(1.0, 1.0, 1.0) * max(0.0, exp(a) - a*Ei(a));
  return scattering_bot + scattering_top;
}

// Cheap ambient color function based on color gradient
vec3 CalcAmbientColor(float h_frac) {
  return mix(vec3(0.4, 0.4, 0.45), vec3(1.0, 1.0, 1.0), h_frac);
}

// Compute the sun color by accounting for extinction due to shadows
float CalcSunExtinction(in vec3 position) {
  // Get Ray from position to sun, and find intersection with atmosphere end
  Ray ray_to_sun = Ray(position, sun_dir);
  float l_ray_to_sun = CalcRayPlaneIntersection(ray_to_sun, cloud_end);
  l_ray_to_sun *= 0.99; // shorten a bit to stay completely inside atmosphere
  // March towards sun and compute extinction
  float extinction = 1.0;
  float step_size = step_size0_over_l * l_ray_to_sun;
  for (int i = 0; i < n_shadow_steps; ++i) {
    position += step_size;
    vec4 w = texture(weather, position.xz * weather_scale);
    float h = max_cloud_height * w.g;
    float y0 = GetCloudStartHeight(h, w.b);
    float h_frac = clamp((position.y - y0) / h, 0.0, 1.0);
    float density = GetDensity(position, w, h_frac);
    float scattering_coeff = sigma_scattering * density;
    float extinction_coeff = max(0.0000001, sigma_extinction * density);
    float step_extinction = exp(-extinction_coeff * step_size);
    extinction *= step_extinction;
    step_size *= step_size_growth;
  }
  return extinction;
}

// Perform ray-marching to compute color and extinction
vec4 RayMarch(Ray ray, vec2 start_stop) {
  // Ray-march to compute scattering and extinction
  int n_steps = 20;
  float step_size = (start_stop.y - start_stop.x) / n_steps;
  float offset = rand(gl_FragCoord.xy) * step_size;
  vec3 position = ray.origin + (start_stop.x + offset) * ray.dir;
  vec3 dposition = step_size * ray.dir;
  float extinction = 1.0;
  vec3 scattering = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < n_steps; ++i) {
    vec4 w = texture(weather, position.xz * weather_scale);
    float h = max_cloud_height * w.g;
    float y0 = GetCloudStartHeight(h, w.b);
    float h_frac = clamp((position.y - y0) / h, 0.0, 1.0);
    float density = GetDensity(position, w, h_frac);
    float scattering_coeff = sigma_scattering * density;
    float extinction_coeff = max(0.0000001, sigma_extinction * density);
    float step_extinction = exp(-extinction_coeff * step_size);

    // Check for early exit due to low transmittance
    if (extinction < 0.01)
      break;
    
    vec3 ambient_color = CalcAmbientColor(h_frac);
    float sun_extinction = CalcSunExtinction(position);
    vec3 step_scattering = scattering_coeff *  
      (CalcSunPhaseFunction(ray.dir) * sun_color * sun_extinction 
      + ambient_color);
    scattering += extinction * 
      (step_scattering - step_scattering * step_extinction) / extinction_coeff;
    extinction *= step_extinction;
    
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
      float intersection_depth = CalcDepth(xyz_int);
      if (intersection_depth < scene_depth) {
        l_stop_march = l_stop;
      }
      else {
        l_stop_march = LinearizeDepth(scene_depth) / 
          LinearizeDepth(intersection_depth) * l_stop;
      }
    }
    else {
      // Ray starts above or below the cloud layer
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
      float intersection_depth = CalcDepth(xyz_int);
      if (intersection_depth < scene_depth) {
        l_start_march = l_start;
      }
      xyz_int = ray.origin + l_stop * ray.dir;
      intersection_depth = CalcDepth(xyz_int);
      if (intersection_depth < scene_depth) {
        l_stop_march = l_stop;
      }
      else {
        l_stop_march = LinearizeDepth(scene_depth) / 
          LinearizeDepth(intersection_depth) * l_stop;
      }
    }
  }
  l_stop_march = min(l_stop_march, l_start_march + l_stop_max);
  return vec2(l_start_march, l_stop_march);
}

// Blend the color obtained by raymarching with the previous frame
vec4 TemporalBlend(vec4 color_in, vec4 eye_pos) {
  // eye_pos *= gl_FragCoord.w; // perspective divide
  vec4 q_cs = projview_prev * eye_pos;
  vec2 q_uv = 0.5 * q_cs.xy / q_cs.w + 0.5;
  if (q_uv.x > 1.0 || q_uv.x < 0.0 || q_uv.y > 1.0 || q_uv.y < 0.0) {
    return color_in;
  }
  else {
    vec4 color_prev = texture(texture_prev, q_uv);
    float alpha = 0.05;
    return alpha * color_in + (1.0 - alpha) * color_prev;
  }
}

void main() {    
  // Compute ray passing through each fragment
  vec4 eye_pos = GetEyeSpacePosition();
  Ray ray = GetFragRay(eye_pos);
  
  // Check where this ray intersects the cloud layer
  vec2 start_stop = GetRayAtmosphereIntersection(ray);

  // Perform ray-marching
  vec4 scatter_extinction = vec4(0.0, 0.0, 0.0, 1.0);
  if (start_stop.x >= 0.0f)
    scatter_extinction = RayMarch(ray, start_stop);

  color = scatter_extinction;
  // color = TemporalBlend(scatter_extinction, eye_pos);
}
