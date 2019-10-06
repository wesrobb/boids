#pragma once

#include "../types.h"

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
    i32 color;
    i32 camPos;

    wr_shdr_uniforms_boids_light light;
    wr_shdr_uniforms_boids_material material;
} wr_shdr_uniforms_boids;

typedef struct wr_shdr_boids {
    u32 program;
    wr_shdr_uniforms_boids uniforms;
} wr_shdr_boids;

typedef struct wr_shdr_boids_data {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    wr_light light;
    wr_material material;
} wr_shdr_boids_data;

wr_shdr_boids wr_shdr_boids_init();
void wr_shdr_boids_update_uniforms(wr_shdr_boids shdr, wr_shdr_boids_data data);
