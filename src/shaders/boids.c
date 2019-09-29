#include "boids.h"

#include "..\opengl.h"

wr_shdr_boids wr_shdr_boids_init()
{
    wr_shdr_boids result = {0};

    u32 vs = CreateVertexShader(VERTEX_SHADER_BOID);
    u32 fs = CreateFragmentShader(FRAGMENT_SHADER_BOID);
    u32 program = CreateShaderProgram(vs, fs);

    glUseProgram(program);
    result.program = program;
    result.uniforms.proj = glGetUniformLocation(program, "projection");
    result.uniforms.view = glGetUniformLocation(program, "view");
    result.uniforms.model = glGetUniformLocation(program, "model");
    result.uniforms.color = glGetUniformLocation(program, "color");
    result.uniforms.camPos = glGetUniformLocation(program, "cameraPos");
    result.uniforms.light.position = glGetUniformLocation(program, "light.position");
    result.uniforms.light.ambient = glGetUniformLocation(program, "light.ambient");
    result.uniforms.light.diffuse = glGetUniformLocation(program, "light.diffuse");
    result.uniforms.light.specular = glGetUniformLocation(program, "light.specular");
    result.uniforms.material.ambient = glGetUniformLocation(program, "material.ambient");
    result.uniforms.material.diffuse = glGetUniformLocation(program, "material.diffuse");
    result.uniforms.material.specular = glGetUniformLocation(program, "material.specular");
    result.uniforms.material.shininess = glGetUniformLocation(program, "material.shininess");
    glUseProgram(0);

    return result;
}

void wr_shdr_boids_update_uniforms(wr_shdr_boids shdr, wr_shdr_boids_data data)
{
    glUseProgram(shdr.program);

    glUniformMatrix4fv(shdr.uniforms.model, 1, GL_FALSE, (GLfloat *)data.model);
    glUniformMatrix4fv(shdr.uniforms.view, 1, GL_FALSE, (GLfloat *)data.view);
    glUniformMatrix4fv(shdr.uniforms.proj, 1, GL_FALSE, (GLfloat *)data.proj);
    glUniform3fv(shdr.uniforms.color, 1, data.color);
    glUniform3fv(shdr.uniforms.camPos, 1, data.camPos);
    glUniform3fv(shdr.uniforms.light.position, 1, data.light.position);
    glUniform3fv(shdr.uniforms.light.ambient, 1, data.light.ambient);
    glUniform3fv(shdr.uniforms.light.diffuse, 1, data.light.diffuse);
    glUniform3fv(shdr.uniforms.light.specular, 1, data.light.specular);
    glUniform3fv(shdr.uniforms.material.ambient, 1, data.material.ambient);
    glUniform3fv(shdr.uniforms.material.diffuse, 1, data.material.diffuse);
    glUniform3fv(shdr.uniforms.material.specular, 1, data.material.specular);
    glUniform1f(shdr.uniforms.material.shininess , data.material.shininess);

    glUseProgram(0);
}
