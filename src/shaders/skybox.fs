#version 330 core

#include "raymarch.glsl"

in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;
uniform float cloud_start;
uniform float cloud_end;

void main() {    
  color = texture(skybox, TexCoords);
}
