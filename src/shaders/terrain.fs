#version 330 core

#include "light.glsl"
#include "material.glsl"
#include "fog.glsl"
#include "shadow.glsl"
#include "noise.glsl"

in vec3 FragPos;  
in vec3 Position;  
in vec3 Normal;  
in vec2 TexCoord;
in vec4 FragPosEyeSpace;
in vec4 FragPosLightSpace[MAX_NUM_CASCADES];
  
out vec4 color;

uniform vec3 viewPos;  
uniform Material material;
uniform Light light;
uniform Fog fog;

void main() {
  // Ambient
  vec3 ambient = CalcAmbient(light.ambient, material.specular);

  // Generate bump map
  /*
  const int oct = 1;
  const float pers = 0.5;
  const float freq = 0.3;
  float h = octave_snoise2D(vec2(Position.x, 0.3*Position.z), oct, pers, freq);
  mat2 duv_dxy;
  duv_dxy[0] = dFdx(Position.xz);
  duv_dxy[1] = dFdy(Position.xz);
  vec2 dh_duv = inverse(duv_dxy) * vec2(dFdx(h), dFdy(h));
  vec3 norm_bump = normalize(vec3(-dh_duv.x, 1.0, -dh_duv.y));
  */
  // TODO
  vec3 norm_bump = vec3(0.0);
 
  // Generate grass texture 
  vec4 grass = vec4(0.2, 0.3, 0.1, 1.0);
  grass.r -= 0.07 * filtered_octave_snoise2D(Position.xz, 2, 0.5, 0.01, 4, 0.125);

  // Add dirt highlights
  vec4 dirt = vec4(0.61, 0.46, 0.33, 1.0);
  dirt -= 0.1 * filtered_octave_snoise2D(Position.xz, 5, 0.5, 0.005, 4, 0.125);
  
  vec3 norm;
  if (dirt.a < 0.95) {
    color = grass;
    norm = normalize(normalize(Normal) + 0.1 * norm_bump);
  }
  else {
    color = dirt;
    norm = normalize(normalize(Normal) + norm_bump);
  }
	
  // Diffuse 
  vec3 lightDir = normalize(-light.direction);  
  vec3 diffuse = CalcDiffuse(norm, lightDir, light.diffuse, material.specular);
  
  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 specular = CalcSpecular(norm, lightDir, viewDir, light.specular, 
    material.specular, material.shiny);
  // TODO
  specular *= 0.0;
 
  // Shadow
  int cascade_idx = GetCascadeIndex(FragPos);
  float shadow = ShadowCalculation(FragPos, FragPosLightSpace[cascade_idx], 
    cascade_idx, shadow_bias[cascade_idx], lightDir, norm);

  color *= vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0f);

  // Fog
  float FogCoord = abs(FragPosEyeSpace.z/FragPosEyeSpace.w);
  color = mix(color, vec4(fog.Color, 1.0f), CalcFogFactor(fog, FogCoord));
} 
