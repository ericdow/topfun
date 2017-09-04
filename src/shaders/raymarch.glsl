uniform ivec4 viewport; // [x0, y0, width, height]
uniform mat4 inv_projview;

struct Ray {
  vec3 origin; // origin
  vec3 dir; // direction
};

// Computes the ray (in world-space) that passes through the current fragment
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
