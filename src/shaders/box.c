#include "box.h"

#include "../opengl.h"

wr_shdr_box wr_shdr_box_init()
{
    wr_shdr_box result = {0};

    u32 boxVertexShader = CreateVertexShader(VERTEX_SHADER_BOX);
    u32 boxFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_BOX);
    u32 boxShaderProgram = CreateShaderProgram(boxVertexShader, boxFragmentShader);

    glUseProgram(boxShaderProgram);
    result.program = boxShaderProgram;
    result.uniforms.proj = glGetUniformLocation(boxShaderProgram, "projection");
    result.uniforms.view = glGetUniformLocation(boxShaderProgram, "view");
    result.uniforms.model = glGetUniformLocation(boxShaderProgram, "model");
    glUseProgram(0);

    return result;
}

void wr_shdr_box_update_uniforms(wr_shdr_box shdr, wr_shdr_box_data data)
{
    glUseProgram(shdr.program);

    glUniformMatrix4fv(shdr.uniforms.model, 1, GL_FALSE, (GLfloat *)data.model);
    glUniformMatrix4fv(shdr.uniforms.view, 1, GL_FALSE, (GLfloat *)data.view);
    glUniformMatrix4fv(shdr.uniforms.proj, 1, GL_FALSE, (GLfloat *)data.proj);

    glUseProgram(0);
}
