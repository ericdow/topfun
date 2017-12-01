#version 330 core

in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;
uniform vec3 fog_color;

void main() {    
  color = texture(skybox, TexCoords);
  // blend in fog
  if (TexCoords.y < 0.0f) {
    float alpha = min(-TexCoords.y * 10, 1.0);
    color = alpha * vec4(fog_color, 1.0) + (1 - alpha) * color;
  }
}
