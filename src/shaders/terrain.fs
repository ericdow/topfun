#version 330 core

#include "light.glsl"
#include "material.glsl"
#include "fog.glsl"
#include "shadow.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoord;
in vec4 FragPosEyeSpace;
in vec4 FragPosLightSpace;
  
out vec4 color;

uniform sampler2D grassTexture0;
uniform sampler2D grassTexture1;
uniform sampler2D grassTexture2;
uniform vec3 viewPos;  
uniform Material material;
uniform Light light;
uniform Fog fog;

void main() {
  // Ambient
  vec3 ambient = CalcAmbient(light.ambient, material.specular);
	
  // Diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.direction);  
  vec3 diffuse = CalcDiffuse(norm, lightDir, light.diffuse, material.specular);
  
  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 specular = CalcSpecular(norm, lightDir, viewDir, light.specular, 
    material.specular, material.shiny);

  vec4 base = texture(grassTexture0, TexCoord);

  vec4 highlight0 = texture(grassTexture1, TexCoord)*pow(abs(norm.z), 1.0f/4) +
    base*(1.0f - pow(abs(norm.z), 1.0f/4));
  
  vec4 highlight1 = texture(grassTexture2, TexCoord)*pow(abs(norm.x), 1.0f/4) +
    base*(1.0f - pow(abs(norm.x), 1.0f/4));
  
  // Shadow
  float shadow = ShadowCalculation(FragPos, FragPosLightSpace, lightDir,
    Normal);
  
  color = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0f)
    * highlight0 * highlight1;

  // TODO 
  // // Fog
  // float FogCoord = abs(FragPosEyeSpace.z/FragPosEyeSpace.w);

  // color = mix(color, vec4(fog.Color, 1.0f), CalcFogFactor(fog, FogCoord));
  
  /////////////////////////////////////////////////////////////////////////
  // perform perspective divide
  vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective 
  // (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(depthMap, projCoords.xy).r; 
  color = vec4(closestDepth);
  // color = vec4(projCoords, 1.0);
  color.w = 1.0; 
  /////////////////////////////////////////////////////////////////////////
} 
