#version 330 core

#include "light.glsl"
#include "material.glsl"
#include "fog.glsl"
#include "shadow.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
in vec4 FragPosEyeSpace;
in vec4 FragPosLightSpace[MAX_NUM_CASCADES];

out vec4 color;

uniform vec3 viewPos;  
uniform Material material;
uniform Light light;
uniform Fog fog;

uniform sampler2D texture_diffuse1;

void main() {    
  vec4 tex = texture(texture_diffuse1, TexCoords);
  
  // Specular
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(-light.direction);  
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 specular = CalcSpecular(norm, lightDir, viewDir, light.specular, 
    material.specular, material.shiny);
  
  // Shadow
  int cascade_idx = GetCascadeIndex(FragPos);
  float shadow = ShadowCalculation(FragPos, FragPosLightSpace[cascade_idx], 
    depthMap[cascade_idx], lightDir, norm);

  color = tex + (1.0 - shadow) * vec4(specular, 1.0f);
  // Set alpha to make transparent
  color.w = 0.9; 
}
