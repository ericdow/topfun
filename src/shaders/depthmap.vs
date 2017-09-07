#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 projection_view; // product of projection and view matrices
uniform mat4 model;

void main() {
  gl_Position = projection_view * model * vec4(position, 1.0);
}
