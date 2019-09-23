#include "types.h"

#include "shaders.h" // SDL redefines main so bring in the shaders before SDL.h

#include "cglm/cglm.h" // Must be included before SDL since SDL won't redefine M_PI. Also brings in stdbool.h
#include "SDL.h"

#include "aabb.h"
#include "boids.h"
#include "camera.h"
#include "opengl.h"

#include <stdio.h>

static char *g_BasePath;
static wr_camera g_camera;

bool SdlProcessEvents(f32 dt)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                wr_camera_movement movement = WR_CAMERA_NONE;
                if (event.key.keysym.sym == SDLK_w)
                    movement |= WR_CAMERA_FORWARD;
                if (event.key.keysym.sym == SDLK_s)
                    movement |= WR_CAMERA_BACKWARD;
                if (event.key.keysym.sym == SDLK_d)
                    movement |= WR_CAMERA_RIGHT;
                if (event.key.keysym.sym == SDLK_a)
                    movement |= WR_CAMERA_LEFT;
                wr_camera_update_keyboard(&g_camera, movement, dt);
            }
            break;
            case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    return true;
                }
            }
            break;
            case SDL_MOUSEMOTION:
            {
                wr_camera_update_mouse(&g_camera, event.motion.xrel, event.motion.yrel);
            }
            break;
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

    u32 width = 1280;
    u32 height = 720;

    // Create an application window with the following settings:
    SDL_Window *window = SDL_CreateWindow(
        "An SDL2 window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL
    );

    SDL_SetRelativeMouseMode(true);

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

    wr_aabb3 bounds = {
        .minX = -5.0f,
        .maxX = 5.0f,
        .minY = -5.0f,
        .maxY = 5.0f,
        .minZ = -5.0f,
        .maxZ = 5.0f,
    };
    wr_boids boids;
    u32 numBoids = 20;
    wr_boids_init(&boids, &bounds, numBoids);
    wr_camera_init(&g_camera, 5.0f, 10.0f, 15.0f);

    vec3 objectColor = { 0.5f, 0.8f, 0.2f };
    vec3 lightColor = { 1.0f, 1.0f, 1.0f };

    wr_material pyramidMaterial = {
        .ambient = {1.0f, 0.5f, 0.31f},
        .diffuse = {1.0f, 0.5f, 0.31f},
        .specular = {0.5f, 0.5f, 0.5f},
        .shininess = 32.0f
    };

    wr_light light = {
        .position = {3.2f, 3.0f, 3.0f},
        .ambient  = {0.2f, 0.2f, 0.2f},
        .diffuse  = {0.5f, 0.5f, 0.5f},
        .specular = {1.0f, 1.0f, 1.0f},
    };

    GLuint objectVertexShader = CreateVertexShader(VERTEX_SHADER_BOID);
    GLuint objectFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_BOID);
    GLuint objectShaderProgram = CreateShaderProgram(objectVertexShader, objectFragmentShader);
    glUseProgram(objectShaderProgram);
    GLint objectProjectionLocation = glGetUniformLocation(objectShaderProgram, "projection");
    GLint objectViewLocation = glGetUniformLocation(objectShaderProgram, "view");
    GLint objectModelLocation = glGetUniformLocation(objectShaderProgram, "model");
    GLint objectColorLocation = glGetUniformLocation(objectShaderProgram, "color");
    glUniform3fv(objectColorLocation, 1, objectColor);

    GLint viewPosLocation = glGetUniformLocation(objectShaderProgram, "viewPos");
    glUniform3fv(viewPosLocation, 1, g_camera.position);

    GLint lightPositionLocation = glGetUniformLocation(objectShaderProgram, "light.position");
    GLint lightAmbientLocation = glGetUniformLocation(objectShaderProgram, "light.ambient");
    GLint lightDiffuseLocation = glGetUniformLocation(objectShaderProgram, "light.diffuse");
    GLint lightSpecularLocation = glGetUniformLocation(objectShaderProgram, "light.specular");
    glUniform3fv(lightPositionLocation, 1, light.position);
    glUniform3fv(lightAmbientLocation, 1, light.ambient);
    glUniform3fv(lightDiffuseLocation, 1, light.diffuse);
    glUniform3fv(lightSpecularLocation, 1, light.specular);

    GLint pyramidAmbientLocation = glGetUniformLocation(objectShaderProgram, "material.ambient");
    GLint pyramidDiffuseLocation = glGetUniformLocation(objectShaderProgram, "material.diffuse");
    GLint objectSpecularLocation = glGetUniformLocation(objectShaderProgram, "material.specular");
    GLint objectShininessLocation = glGetUniformLocation(objectShaderProgram, "material.shininess");
    glUniform3fv(pyramidAmbientLocation, 1, pyramidMaterial.ambient);
    glUniform3fv(pyramidDiffuseLocation, 1, pyramidMaterial.diffuse);
    glUniform3fv(objectSpecularLocation, 1, pyramidMaterial.specular);
    glUniform1f(objectShininessLocation , pyramidMaterial.shininess);

    GLuint lightVertexShader = CreateVertexShader(VERTEX_SHADER_LIGHT);
    GLuint lightFragmentShader = CreateFragmentShader(FRAGMENT_SHADER_LIGHT);
    GLuint lightShaderProgram = CreateShaderProgram(lightVertexShader, lightFragmentShader);
    glUseProgram(lightShaderProgram);
    GLint lightProjectionLocation = glGetUniformLocation(lightShaderProgram, "projection");
    GLint lightViewLocation = glGetUniformLocation(lightShaderProgram, "view");
    GLint lightModelLocation = glGetUniformLocation(lightShaderProgram, "model");
    GLint lightColorLocation = glGetUniformLocation(lightShaderProgram, "color");
    glUniform3fv(lightColorLocation, 1, lightColor);

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

    GLuint instanceBuffer;
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * numBoids, &boids.positions[0], GL_STATIC_DRAW);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

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

        stopping = SdlProcessEvents(dt);


        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(objectShaderProgram);
        mat4 objectModel = GLM_MAT4_IDENTITY_INIT;
        //glm_rotate(objectModel, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f});
        glUniformMatrix4fv(objectModelLocation, 1, GL_FALSE, (GLfloat *)objectModel);

        mat4 view;
        wr_camera_view(&g_camera, view);
        glUniformMatrix4fv(objectViewLocation, 1, GL_FALSE, (GLfloat *)view);

        mat4 proj;
        glm_perspective(glm_rad(45.0f), (f32)width / (f32)height, 0.1f, 100.0f, proj);
        glUniformMatrix4fv(objectProjectionLocation, 1, GL_FALSE, (GLfloat *)proj);

        f32 radius = 5.0f;
        light.position[0] = sinf((f32)SDL_GetTicks() / 5000.0f) * radius;
        light.position[2] = cosf((f32)SDL_GetTicks() / 5000.0f) * radius;
        glUniform3fv(lightPositionLocation, 1, light.position);

        glUseProgram(objectShaderProgram);
        glBindVertexArray(pyramidVao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, numBoids);

        glUseProgram(lightShaderProgram);

        mat4 lightModel;
        glm_mat4_identity(lightModel);
        glm_translate(lightModel, light.position);
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

