uniform ivec4 viewport; // [x0, y0, width, height]
uniform mat4 projview; // product of projection and view matrices
uniform mat4 inv_projview; // inverse of above
uniform float camera_near; // near plane distance
uniform float camera_far; // far plane distance

struct Ray {
  vec3 origin; // origin
  vec3 dir; // direction
};

// Computes the ray (in world-space) that passes through the current fragment
// The direction is stored as a vector pointing from the near point to the far
Ray GetFragRay() {
  vec4 near = vec4(
  2.0 * ( (gl_FragCoord.x - viewport[0]) / viewport[2] - 0.5),
  2.0 * ( (gl_FragCoord.y - viewport[1]) / viewport[3] - 0.5),
  0.0,
  1.0);
  near = inv_projview * near;
  vec4 far = near + inv_projview[2];
  near.xyz /= near.w;
  far.xyz /= far.w;
  return Ray(near.xyz, far.xyz-near.xyz);
}

// Computes the t-coordinate of intersection with a ray and a play at height y
// t = 0 corresponds to the near point of the ray
// t = 1 corresponds to the far point of the ray
float CalcRayAtmosphereIntersection(Ray ray, float y) {
  return (y - ray.origin.y) / ray.dir.y;
}

// Computes the depth coordinate of an arbitrary point in world-space coordinates
float CalcDepth(vec3 v_world_space) {
  vec4 v_clip_space = projview * vec4(v_world_space, 1.0);
  float z = 0.5 * (1.0 + v_clip_space.z / v_clip_space.w);
  return (1.0 - z) * camera_near + z * camera_far;
}
