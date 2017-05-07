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

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
out vec4 color;

uniform vec3 viewPos;  
uniform Material material;
uniform Light light;

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
          
  color = vec4(ambient + diffuse + specular, 1.0f);  
} 
