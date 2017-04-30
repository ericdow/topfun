#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 color;
  
uniform vec3 objectColor; // color of the object
uniform vec3 lightColor; // color of the light shining on object
uniform vec3 lightPos;

void main()
{
    // calculate the ambient contribution
    float ambientStrength = 0.4f;
    vec3 ambient = ambientStrength * lightColor;

    // calculate the diffuse contribution
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
   
    // combine the light contributions 
    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);
}
