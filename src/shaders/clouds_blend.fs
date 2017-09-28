#version 330 core

in vec2 TexCoord;

uniform sampler2D clouds;

out vec4 color;

void main() {             
  color = texture(clouds, TexCoord);
}
