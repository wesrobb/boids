#pragma once

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

wr_shdr_light wr_shdr_light_init();
