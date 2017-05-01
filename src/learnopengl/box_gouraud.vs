#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;
out vec3 FragPos;
out float ads;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos; // position of the light source
uniform vec3 viewPos; // position of the camera

void main()
{
    // Note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(position, 1.0f);
    // TODO: This is slow, probably should be done in geometry shader
    Normal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(model * vec4(position, 1.0f));
    
    // calculate the ambient contribution
    float ambientStrength = 0.4f;
    float ambient = ambientStrength;

    // calculate the diffuse contribution
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float diffuse = diff;

    // calculate the specular contribution
    float specularStrength = 0.2f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specular = specularStrength * spec;
    
    // intensity scales like 1 / r^2
    float intensity = 10.0f / pow(length(abs(lightPos - FragPos)), 2);

    ads = intensity * (ambient + diffuse + specular); 
}
