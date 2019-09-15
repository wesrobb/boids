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

#include "shaders.c" // This must be included before SDL.h because SDL redefines main and main is used in GLSL

#include <stdio.h>
#include "cglm/cglm.h" // Must be included before SDL since SDL won't redefine M_PI. Also brings in stdbool.h
#include "SDL.h"

#include "wr_opengl.c"

static char *g_BasePath;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

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

GLuint CreateVertexShader(const char* vertexShaderSource)
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
        printf("Vertex shader compilation failed: %s\n", log);
        exit(1);
    }

    return vertexShader;
}

GLuint CreateFragmentShader(const char* fragmentShaderSource)
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

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void CalculateNormals(vec3 *vertices, u32 verticesCount, vec3 *normals, u32 normalsCount)
{
    SDL_assert(vertices);
    SDL_assert(normals);
    SDL_assert(verticesCount % 3 == 0);
    SDL_assert(normalsCount == verticesCount);

    u32 normalIndex = 0;
    vec3 u, v, cross;
    for (u32 i = 0; i < verticesCount; i+=3)
    {
        vec3 *x = &vertices[i];
        vec3 *y = &vertices[i+1];
        vec3 *z = &vertices[i+2];

        glm_vec3_sub(*x, *y, u);
        glm_vec3_sub(*x, *z, v);
        glm_vec3_cross(u, v, cross);
        glm_vec3_normalize(cross);

        // Store a normal for each of the vertices
        glm_vec3_copy(cross, normals[normalIndex++]);
        glm_vec3_copy(cross, normals[normalIndex++]);
        glm_vec3_copy(cross, normals[normalIndex++]);
    }
}

int SDL_main(int argc, char ** argv)
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

    vec3 objectColor = { 0.5f, 0.8f, 0.2f };
    vec3 lightColor = { 1.0f, 1.0f, 1.0f };
    vec3 lightPos = {3.2f, 3.0f, 3.0f};

    GLuint objectVertexShader = CreateVertexShader(VERTEX_SHADER_PYRAMID);
    GLuint objectFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_PYRAMID);
    GLuint objectShaderProgram = CreateShaderProgram(objectVertexShader, objectFragmentShader);
    glUseProgram(objectShaderProgram);
    GLint objectProjectionLocation = glGetUniformLocation(objectShaderProgram, "projection");
    GLint objectViewLocation = glGetUniformLocation(objectShaderProgram, "view");
    GLint objectModelLocation = glGetUniformLocation(objectShaderProgram, "model");
    GLint objectColorLocation = glGetUniformLocation(objectShaderProgram, "color");
    GLint objectLightColorLocation = glGetUniformLocation(objectShaderProgram, "lightColor");
    GLint lightPosLocation = glGetUniformLocation(objectShaderProgram, "lightPos");
    glUniform3fv(objectColorLocation, 1, objectColor);
    glUniform3fv(objectLightColorLocation, 1, lightColor);
    glUniform3fv(lightPosLocation, 1, lightPos);

    GLuint lightVertexShader = CreateVertexShader(VERTEX_SHADER_LIGHT);
    GLuint lightFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_LIGHT);
    GLuint lightShaderProgram = CreateShaderProgram(lightVertexShader, lightFragmentShader);
    glUseProgram(lightShaderProgram);
    GLint lightProjectionLocation = glGetUniformLocation(lightShaderProgram, "projection");
    GLint lightViewLocation = glGetUniformLocation(lightShaderProgram, "view");
    GLint lightModelLocation = glGetUniformLocation(lightShaderProgram, "model");
    GLint lightColorLocation = glGetUniformLocation(lightShaderProgram, "color");
    glUniform3fv(lightColorLocation, 1, lightColor);

    f32 camX = 0.0f, camZ = 0.0f;

    vec3 pyramidVertices[] = {
        { 0.0f, 1.0f, 0.0f},
        {-1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f, 1.0f},
        { 0.0f, 1.0f, 0.0f},
        { 1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 0.0f, 1.0f, 0.0f},
        { 1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f,-1.0f},
        { 0.0f, 1.0f, 0.0f},
        {-1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f, 1.0f},
    };

    const u32 normalsCount = ARRAY_SIZE(pyramidVertices);
    vec3 pyramidNormals[ARRAY_SIZE(pyramidVertices)];
    CalculateNormals(pyramidVertices, ARRAY_SIZE(pyramidVertices), pyramidNormals, normalsCount);

    GLuint pyramidVao;
    glGenVertexArrays(1, &pyramidVao);
    glBindVertexArray(pyramidVao);

    GLuint pyramidVertexBuffer;
    glGenBuffers(1, &pyramidVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint pyramidNormalsBuffer;
    glGenBuffers(1, &pyramidNormalsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidNormalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    GLuint lightVao;
    glGenVertexArrays(1, &lightVao);
    glBindVertexArray(lightVao);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVertexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    u64 now = SDL_GetPerformanceCounter();
    u64 last = 0;
    f32 dtTotal = 0.0f;
    u32 frameCounter = 0;

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

        glUseProgram(objectShaderProgram);
        mat4 objectModel;
        glm_mat4_identity(objectModel);
        glm_rotate(objectModel, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f});
        glUniformMatrix4fv(objectModelLocation, 1, GL_FALSE, (GLfloat *)objectModel);

        f32 radius = 10.0f;
        camX = sinf((f32)SDL_GetTicks() / 5000.0f) * radius;
        camZ = cosf((f32)SDL_GetTicks() / 5000.0f) * radius;

        mat4 view;
        glm_lookat((vec3){camX, 0.0f, camZ},
                   (vec3){0.0f, 0.0f, 0.0f},
                   (vec3){0.0f, 1.0f, 0.0f},
                   view);
        glUniformMatrix4fv(objectViewLocation, 1, GL_FALSE, (GLfloat *)view);

        mat4 proj;
        glm_perspective(glm_rad(45.0f), (f32)width / (f32)height, 0.1f, 100.0f, proj);
        glUniformMatrix4fv(objectProjectionLocation, 1, GL_FALSE, (GLfloat *)proj);

        glUseProgram(objectShaderProgram);
        glBindVertexArray(pyramidVao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(lightShaderProgram);

        mat4 lightModel;
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, lightPos);
        //glm_scale(lightModel, (vec3){0.2f, 0.2f, 0.2f});
        glUniformMatrix4fv(lightModelLocation, 1, GL_FALSE, (GLfloat *)lightModel);
        glUniformMatrix4fv(lightViewLocation, 1, GL_FALSE, (GLfloat *)view);
        glUniformMatrix4fv(lightProjectionLocation, 1, GL_FALSE, (GLfloat *)proj);

        glBindVertexArray(lightVao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

