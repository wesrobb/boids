#pragma once

#include "../types.h"

#define SHADER(name, prefix, version, shaderSource) static const char* prefix##name = "#version " #version "\n" #shaderSource
#define VERTEX_SHADER(name, shaderSource) SHADER(name, VERTEX_SHADER_, 330, shaderSource)
#define FRAGMENT_SHADER(name, shaderSource) SHADER(name, FRAGMENT_SHADER_, 330, shaderSource)

u32 CreateVertexShader(const char* vertexShaderSource);
u32 CreateFragmentShader(const char* fragmentShaderSource);
u32 CreateShaderProgram(u32 vertexShader, u32 fragmentShader);
