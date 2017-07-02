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

  color = vec4(ambient + diffuse + specular, 1.0f);
}
