typedef char               i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef float              f32;
typedef double             f64;

#include <stdio.h>
#include <string.h>
#include "cglm/cglm.h" // Must be included before SDL since SDL won't redefine M_PI. Also brings in stdbool.h
#include "SDL.h"

#include "wr_opengl.c"

static char *g_BasePath;

bool SdlProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool SdlReadFile(const char *filePath, char *buffer, u32 bufferSize)
{
    SDL_RWops *rw = SDL_RWFromFile(filePath, "rb");
    if (!rw)
    {
        return false;
    }

    i64 fileSize = SDL_RWsize(rw);
    if (fileSize + 1 > bufferSize)
    {
        return false;
    }

    i64 totalBytesRead = 0;
    i64 bytesRead = 1;
    while (totalBytesRead < fileSize && bytesRead != 0)
    {
        bytesRead = SDL_RWread(rw, buffer, 1, fileSize - totalBytesRead);
        totalBytesRead += bytesRead;
        buffer += bytesRead;
    }

    SDL_RWclose(rw);
    if (totalBytesRead != fileSize)
    {
        return false;
    }

    buffer[totalBytesRead] = 0;
    return true;
}

GLuint CreateVertexShader(const char* vertexShaderFileName)
{
    char vertexShaderPath[512] = {0};
    strcpy_s(vertexShaderPath, 512, g_BasePath);
    strcat_s(vertexShaderPath, 512, vertexShaderFileName);

    char vertexShaderSource[2048] = {0};
    SdlReadFile(vertexShaderPath, vertexShaderSource, 2048);

    const char *src = vertexShaderSource;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &src, 0);
    glCompileShader(vertexShader);
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(vertexShader, 512, 0, log);
        printf("Vertex shader compilation failed: %s\n", log);
        exit(1);
    }

    return vertexShader;
}

GLuint CreateFragmentShader(const char* fragmentShaderFileName)
{
    char fragmentShaderPath[512] = {0};
    strcpy_s(fragmentShaderPath, 512, g_BasePath);
    strcat_s(fragmentShaderPath, 512, fragmentShaderFileName);

    char fragmentShaderSource[2048] = {0};
    SdlReadFile(fragmentShaderPath, fragmentShaderSource, 2048);

    const char *src = fragmentShaderSource;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &src, 0);
    glCompileShader(fragmentShader);
    int success;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(fragmentShader, 512, 0, log);
        printf("Fragment shader compilation failed: %s\n", log);
        exit(1);
    }

    return fragmentShader;
}

GLuint CreateShaderProgram(GLuint vertexShader, GLuint fragmentShader)
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
        printf("Shader program linking failed: %s\n", log);
        exit(1);
    }

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main(int argc, char ** argv)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Init(SDL_INIT_EVERYTHING);         // Initialize SDL2

    g_BasePath = SDL_GetBasePath();

    u32 width = 640;
    u32 height = 480;

    // Create an application window with the following settings:
    SDL_Window *window = SDL_CreateWindow(
        "An SDL2 window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL
    );

    // Check that the window was successfully created
    if (window == 0)
    {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    wr_opengl_init();
    printf("OpenGL version is (%s)\n", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);

    GLuint vertexShader = CreateVertexShader("vertex.glsl");
    GLuint fragmentShader = CreateFragmentShader("fragment.glsl");
    GLuint shaderProgram = CreateShaderProgram(vertexShader, fragmentShader);

    GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
    GLint modelLocation = glGetUniformLocation(shaderProgram, "model");

    u64 now = SDL_GetPerformanceCounter();
    u64 last = 0;
    f32 dtTotal = 0.0f;
    u32 frameCounter = 0;

    f32 camX = 0.0f, camZ = 0.0f;

    f32 pyramidVertices[] = {
         0.0f, 1.0f, 0.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         1.0f,-1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         0.0f, 1.0f, 0.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
         0.0f, 1.0f, 0.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
    };

    f32 pyramidColors[] = {
        1.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,
        0.0f,1.0f,0.0f,
        1.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,
        0.0f,1.0f,0.0f,
        1.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,0.0f,1.0f,
        1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,
        0.0f,1.0f,0.0f,
    };

    bool stopping = false;
    while (!stopping)
    {
        stopping = SdlProcessEvents();

        frameCounter++;
        last = now;
        now = SDL_GetPerformanceCounter();
        f32 dt = ((now - last)*1000 / (f32)SDL_GetPerformanceFrequency());
        dtTotal += dt;
        if (dtTotal > 1000.0f)
        {
            printf("Avg frame time %f\n", dtTotal / (f32)frameCounter);
            printf("Avg fps %d\n", frameCounter);
            dtTotal = 0.0f;
            frameCounter = 0;
        }

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 model;
        glm_mat4_identity(model);
        glm_rotate(model, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f});
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat *)model);

        f32 radius = 10.0f;
        camX = sinf((f32)SDL_GetTicks() / 1000.0f) * radius;
        camZ = cosf((f32)SDL_GetTicks() / 1000.0f) * radius;

        mat4 view;
        glm_lookat((vec3){camX, 0.0f, camZ},
                   (vec3){0.0f, 0.0f, 0.0f},
                   (vec3){0.0f, 1.0f, 0.0f},
                   view);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat *)view);

        mat4 proj;
        glm_perspective(glm_rad(45.0f), (f32)width / (f32)height, 0.1f, 100.0f, proj);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (GLfloat *)proj);

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        GLuint colorBuffer;
        glGenBuffers(1, &colorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 18);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

