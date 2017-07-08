#version 330 core

#include "fog.glsl"
#include "light.glsl"

in vec4 EyeSpacePos;

out vec4 color;

uniform vec4 exhaust_color;
uniform Fog fog;

void main() {
  color = exhaust_color;
}
