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
