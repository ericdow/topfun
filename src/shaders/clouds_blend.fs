#version 330 core

in vec2 TexCoord;

uniform sampler2D clouds;
uniform sampler2D scene_depth; // depth map of scene
uniform sampler2D depth_curr; // depth map of clouds

out vec4 color;

void main() { 
  if (texture(scene_depth, TexCoord).r > texture(depth_curr, TexCoord).r) {
    color = texture(clouds, TexCoord);
  }
  else {
    color = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
