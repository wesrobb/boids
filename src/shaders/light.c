#include "light.h"

#include "../opengl.h"

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
