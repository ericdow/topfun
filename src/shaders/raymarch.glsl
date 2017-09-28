uniform ivec4 viewport; // [x0, y0, width, height]
uniform mat4 projview; // product of projection and view matrices
uniform mat4 inv_projview; // inverse of above
uniform mat4 projview_prev; // projview from previous render
uniform float camera_near; // near plane distance
uniform float camera_far; // far plane distance
uniform float l_stop_max; // max marching distance
uniform float seed; // seed for random number generator

struct Ray {
  vec3 origin; // origin
  vec3 dir; // direction (normalized)
};

// Compute the eye-space position of a fragment (not normalized by w)
// See https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
vec4 GetEyeSpacePosition() {
  vec4 tmp = vec4(
  2.0 * ( (gl_FragCoord.x - viewport[0]) / viewport[2] - 0.5),
  2.0 * ( (gl_FragCoord.y - viewport[1]) / viewport[3] - 0.5),
  0.0,
  1.0);
  return inv_projview * tmp;
}

// Computes the ray (in world-space) that passes through the current fragment
// The direction is stored as a vector pointing from the near point to the far
Ray GetFragRay(vec4 eye_pos) {
  vec4 near = eye_pos;
  vec4 far = near + inv_projview[2];
  near.xyz /= near.w;
  far.xyz /= far.w;
  return Ray(near.xyz, normalize(far.xyz-near.xyz));
}

// Computes the distance of intersection of a ray and a plane at height y
float CalcRayPlaneIntersection(Ray ray, float y) {
  return (y - ray.origin.y) / ray.dir.y;
}

// Computes depth coordinate of an arbitrary point in world-space coordinates
// returned value is in [0,1]
float CalcDepth(vec3 v_world_space) {
  vec4 v_clip_space = projview * vec4(v_world_space, 1.0);
  return 0.5 * (1.0 + v_clip_space.z / v_clip_space.w);
}

float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * camera_near * camera_far) / 
    (camera_far + camera_near - z * (camera_far - camera_near));  
}

// Fast(ish) random number generator that returns values in [0,1]
float rand(vec2 xy) {
  return fract(sin(dot(seed * xy, vec2(12.9898, 78.233))) * 43758.5453);
}
