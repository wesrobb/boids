#include "light.h"

#include "shaders.h"
#include "../opengl.h"

VERTEX_SHADER(LIGHT,
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
});

FRAGMENT_SHADER(LIGHT,
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
});

wr_shdr_light wr_shdr_light_init()
{
    wr_shdr_light result = {0};

    u32 lightVertexShader = CreateVertexShader(VERTEX_SHADER_LIGHT);
    u32 lightFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_LIGHT);
    u32 lightShaderProgram = CreateShaderProgram(lightVertexShader, lightFragmentShader);

    glUseProgram(lightShaderProgram);
    result.program = lightShaderProgram;
    result.uniforms.proj = glGetUniformLocation(lightShaderProgram, "projection");
    result.uniforms.view = glGetUniformLocation(lightShaderProgram, "view");
    result.uniforms.model = glGetUniformLocation(lightShaderProgram, "model");
    result.uniforms.color = glGetUniformLocation(lightShaderProgram, "color");
    glUseProgram(0);

    return result;
}

void wr_shdr_light_update_uniforms(wr_shdr_light shdr, wr_shdr_light_data data)
{
    glUseProgram(shdr.program);

    glUniformMatrix4fv(shdr.uniforms.model, 1, GL_FALSE, (GLfloat *)data.model);
    glUniformMatrix4fv(shdr.uniforms.view, 1, GL_FALSE, (GLfloat *)data.view);
    glUniformMatrix4fv(shdr.uniforms.proj, 1, GL_FALSE, (GLfloat *)data.proj);
    glUniform3fv(shdr.uniforms.color, 1, data.color);

    glUseProgram(0);
}
