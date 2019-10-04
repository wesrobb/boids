#include "boids.h"

#include "shaders.h"
#include "..\opengl.h"

#undef main // SDL redefines main but our shader code needs it

VERTEX_SHADER(BOID,
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in mat4 model;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * norm; // Do this on the CPU and pass in via uniform
    gl_Position = projection * view * model * vec4(pos, 1.0);
});


FRAGMENT_SHADER(BOID,
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 color;
uniform vec3 cameraPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
});

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
