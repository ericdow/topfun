#version 330 core
in vec3 Normal;
in vec3 FragPos;
in float ads; // sum of ambient, diffuse and specular

out vec4 color;
  
uniform vec3 objectColor; // color of the object
uniform vec3 lightColor; // color of the light shining on object

void main()
{
    vec3 result = ads * lightColor * objectColor;
    color = vec4(result, 1.0f);
}
