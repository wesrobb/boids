#pragma once

#include "../types.h"
#include "shaders.h"

VERTEX_SHADER(BOX,
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
});

FRAGMENT_SHADER(BOX,
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.5, 0.5, 0.5, 0.5);
});

typedef struct wr_shdr_uniforms_box {
    i32 proj;
    i32 view;
    i32 model;
} wr_shdr_uniforms_box;

typedef struct wr_shdr_box {
    u32 program;
    wr_shdr_uniforms_box uniforms;
} wr_shdr_box;

typedef struct wr_shdr_box_data  {
    mat4 model;
    mat4 view;
    mat4 proj;
} wr_shdr_box_data;

wr_shdr_box wr_shdr_box_init();
void wr_shdr_box_update_uniforms(wr_shdr_box shdr, wr_shdr_box_data data);
