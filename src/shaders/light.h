#pragma once

#include "../types.h"
#include "shaders.h"

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

typedef struct wr_shdr_uniforms_light {
    i32 proj;
    i32 view;
    i32 model;
    i32 color;
} wr_shdr_uniforms_light;

typedef struct wr_shdr_light {
    u32 program;
    wr_shdr_uniforms_light uniforms;
} wr_shdr_light;

typedef struct wr_shdr_light_data  {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
} wr_shdr_light_data;

wr_shdr_light wr_shdr_light_init();
void wr_shdr_light_update_uniforms(wr_shdr_light shdr, wr_shdr_light_data data);
