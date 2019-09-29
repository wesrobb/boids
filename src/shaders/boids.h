#pragma once

#include "shaders.h"

VERTEX_SHADER(BOID,
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 offset;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    FragPos = vec3(model * vec4(pos + offset, 1.0));
    Normal = mat3(transpose(inverse(model))) * norm; // Do this on the CPU and pass in via uniform
    gl_Position = projection * view * model * vec4(pos + offset, 1.0);
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
uniform vec3 camPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
});

typedef struct wr_shdr_uniforms_boids_light {
    i32 position;
    i32 ambient;
    i32 diffuse;
    i32 specular;
} wr_shdr_uniforms_boids_light ;

typedef struct wr_shdr_uniforms_boids_material {
    i32 ambient;
    i32 diffuse;
    i32 specular;
    i32 shininess;
} wr_shdr_uniforms_boids_material;

typedef struct wr_shdr_uniforms_boids {
    i32 proj;
    i32 view;
    i32 model;
    i32 color;
    i32 camPos;

    wr_shdr_uniforms_boids_light light;
    wr_shdr_uniforms_boids_material material;
} wr_shdr_uniforms_boids;

typedef struct wr_shdr_boids {
    u32 program;
    wr_shdr_uniforms_boids uniforms;
} wr_shdr_boids;

wr_shdr_boids wr_shdr_boids_init();
