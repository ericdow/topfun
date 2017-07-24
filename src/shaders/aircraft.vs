#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosEyeSpace;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {
  FragPosEyeSpace = view * model * vec4(position, 1.0f);
  gl_Position = projection * FragPosEyeSpace;
  FragPos = vec3(model * vec4(position, 1.0f));
  Normal = mat3(transpose(inverse(model))) * normal;
  TexCoords = texCoords;
  FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
