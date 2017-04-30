#version 330 core
out vec4 color;
  
uniform vec3 objectColor; // color of the object
uniform vec3 lightColor; // color of the light shining on object

void main()
{
    color = vec4(lightColor * objectColor, 1.0f);
}
