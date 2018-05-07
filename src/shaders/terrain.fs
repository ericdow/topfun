#version 330 core

#include "light.glsl"
#include "material.glsl"
#include "fog.glsl"
#include "shadow.glsl"

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoord;
in vec4 FragPosEyeSpace;
in vec4 FragPosLightSpace[MAX_NUM_CASCADES];
  
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
  vec3 lightDir = normalize(-light.direction);  
  vec3 diffuse = CalcDiffuse(norm, lightDir, light.diffuse, material.specular);
  
  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 specular = CalcSpecular(norm, lightDir, viewDir, light.specular, 
    material.specular, material.shiny);

  float highlight0 = 1.0f; 
  vec4 highlight1 = texture(grassTexture2, TexCoord);

  // TODO use normals to set weights
  float wt = max(1.0, 3 * texture(grassTexture2, 0.0625 * TexCoord).r);
  wt *= 20.0 * (norm.y - 0.95);
  // float wt2 = max(1.0, 3 * texture(grassTexture1, 0.1 * TexCoord).g);
  highlight1 = wt * highlight1 + (1.0 - wt) * texture(grassTexture1, TexCoord);
  // highlight1 = wt * highlight1 + wt2 * texture(grassTexture0, TexCoord);
  //   (1.0 - wt - wt2) * texture(grassTexture1, TexCoord);
  // highlight1 *= 0.666;
  
  highlight1 = texture(grassTexture0, TexCoord);
  highlight1.rgb *= texture(grassTexture0, -0.25 * TexCoord).rgb;
  highlight1.rgb *= texture(grassTexture0, -0.09 * TexCoord).rgb;
 
  // Shadow
  int cascade_idx = GetCascadeIndex(FragPos);
  float shadow = ShadowCalculation(FragPos, FragPosLightSpace[cascade_idx], 
    cascade_idx, shadow_bias[cascade_idx], lightDir, norm);
  
  color = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0f)
    * highlight0 * highlight1;

  // Fog
  float FogCoord = abs(FragPosEyeSpace.z/FragPosEyeSpace.w);

  color = mix(color, vec4(fog.Color, 1.0f), CalcFogFactor(fog, FogCoord));
} 
