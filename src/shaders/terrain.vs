#version 330 core

#include "shadow.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;
out vec4 FragPosEyeSpace;
out vec4 FragPosLightSpace[MAX_NUM_CASCADES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  FragPosEyeSpace = view * model * vec4(position, 1.0f);
  gl_Position = projection * FragPosEyeSpace;
  FragPos = vec3(model * vec4(position, 1.0f));
  Normal = normal;  
	TexCoord = texCoord;
  for (int i = 0; i < num_cascades; ++i) {
    FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos, 1.0);
  }
} 
