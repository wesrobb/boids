#version 330 core

layout (location = 0) in vec3 pos;

out vec4 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    gl_Position = projection * view * model * vec4(pos, 1.0);
}
