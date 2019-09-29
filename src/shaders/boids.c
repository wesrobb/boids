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
    i32 color = glGetUniformLocation(program, "color");
    i32 camPos = glGetUniformLocation(program, "camPos");
    result.uniforms.light.position = glGetUniformLocation(program, "light.position");
    result.uniforms.light.ambient = glGetUniformLocation(program, "light.ambient");
    result.uniforms.light.diffuse = glGetUniformLocation(program, "light.diffuse");
    result.uniforms.light.specular = glGetUniformLocation(program, "light.specular");
    result.uniforms.material.ambient = glGetUniformLocation(program, "material.ambient");
    result.uniforms.material.diffuse = glGetUniformLocation(program, "material.diffuse");
    result.uniforms.material.specular = glGetUniformLocation(program, "material.specular");
    result.uniforms.material.shininess = glGetUniformLocation(program, "material.shininess");
    result.uniforms.color = color;
    result.uniforms.camPos = camPos;
    glUseProgram(0);

    return result;
}
