#version 330 core

#include "material.glsl"
#include "light.glsl"
#include "fog.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
in vec4 EyeSpacePos;

out vec4 color;

uniform vec3 viewPos;  
uniform Material material;
uniform Light light;
uniform Fog fog;
uniform sampler2D texture_diffuse1;
uniform vec3 flame_color;
uniform vec3 flame1_pos;
uniform vec3 flame2_pos;
uniform float r_flame;

void main() {    
  vec3 tex = texture(texture_diffuse1, TexCoords).rgb;

  // Ambient
  vec3 ambient = CalcAmbient(light.ambient, tex);
  
  // Diffuse
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(-light.direction);  
  vec3 diffuse = CalcDiffuse(norm, lightDir, light.diffuse, tex);
  
  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 specular = CalcSpecular(norm, lightDir, viewDir, light.specular, 
    material.specular, material.shiny);

  // Engine flame
  float d2_flame1 = dot(FragPos - flame1_pos, FragPos - flame1_pos);
  float d2_flame2 = dot(FragPos - flame2_pos, FragPos - flame2_pos);
  float dot1 = dot(norm, flame1_pos - FragPos);
  float dot2 = dot(norm, flame2_pos - FragPos);
  if ((d2_flame1 < r_flame*r_flame || d2_flame2 < r_flame*r_flame) &&
      (dot1 > 0.0f || dot2 > 0.0f)) {
    specular *= 0.0f;
    float alpha = exp(-d2_flame1/0.12f) + exp(-d2_flame2/0.12f);
    ambient = alpha * flame_color + (1.0f - alpha) * ambient;
  }

  color = vec4(ambient + diffuse + specular, 1.0f);
}
