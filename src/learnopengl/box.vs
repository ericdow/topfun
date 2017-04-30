#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(position, 1.0f);
    // TODO: This is slow, probably should be done in geometry shader
    Normal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(model * vec4(position, 1.0f));
}
