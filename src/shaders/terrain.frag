#version 330 core
struct Material {
  vec3 color; 
  float shininess;
};  

struct Light {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Fog {
  vec3 Color;
  float Start;
  float End;
  float Density;
  int Equation; // 0 = linear, 1 = exp, 2 = exp2
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoord;
in vec4 EyeSpacePos;
  
out vec4 color;

uniform sampler2D grassTexture0;
uniform sampler2D grassTexture1;
uniform sampler2D grassTexture2;
uniform vec3 viewPos;  
uniform Material material;
uniform Light light;
uniform Fog fog;

float CalcFogFactor(Fog params, float FogCoord) 
{
  float Result = 0.0;
  switch(params.Equation) {
    case 0:
      Result = (params.End - FogCoord)/(params.End - params.Start);
      break;
    case 1:
      Result = exp(-params.Density*FogCoord);
      break;
    case 2:
      Result = exp(-params.Density*FogCoord*params.Density*FogCoord);
      break;
    default:
      break;
  }
  Result = 1.0 - clamp(Result, 0.0, 1.0);
  return Result;
}

void main()
{
  // Ambient
  vec3 ambient = light.ambient * material.color;
	
  // Diffuse 
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(-light.direction);  
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * material.color;
  
  // Specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm); 
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * material.color;

  vec4 base = texture(grassTexture0, TexCoord);

  vec4 highlight0 = texture(grassTexture1, TexCoord)*pow(abs(norm.z),1.0f/4) + 
    base*(1.0f - pow(abs(norm.z),1.0f/4));
  
  vec4 highlight1 = texture(grassTexture2, TexCoord)*pow(abs(norm.x),1.0f/4) + 
    base*(1.0f - pow(abs(norm.x),1.0f/4));
  
  color = vec4(ambient + diffuse + specular, 1.0f) * highlight0 * highlight1;
 
  // Add Fog
  float FogCoord = abs(EyeSpacePos.z/EyeSpacePos.w);
  color = mix(color, vec4(fog.Color, 1.0f), CalcFogFactor(fog, FogCoord));
} 
