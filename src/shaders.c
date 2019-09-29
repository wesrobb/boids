#include "shaders.h"

#include "opengl.h"
#include "types.h"

#include "SDL_log.h"

#include "stdlib.h"

u32 CreateVertexShader(const char* vertexShaderSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, 0);
    glCompileShader(vertexShader);
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(vertexShader, 512, 0, log);
        SDL_Log("Vertex shader compilation failed: %s\n", log);
        exit(1); // TODO: Get rid of this
    }

    return vertexShader;
}

u32 CreateFragmentShader(const char* fragmentShaderSource)
{
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, 0);
    glCompileShader(fragmentShader);
    int success;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(fragmentShader, 512, 0, log);
        SDL_Log("Fragment shader compilation failed: %s\n", log);
        exit(1); // TODO: Get rid of this
    }

    return fragmentShader;
}

u32 CreateShaderProgram(u32 vertexShader, u32 fragmentShader)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    int success;
    char log[512] = {0};
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, 0, log);
        SDL_Log("Shader program linking failed: %s\n", log);
        exit(1); // TODO: Get rid of this
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

