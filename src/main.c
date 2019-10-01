#include "types.h"

// SDL redefines main so bring in the shaders before SDL.h
#include "shaders/boids.h"
#include "shaders/box.h"
#include "shaders/light.h"

#include "SDL.h"
#include "cglm/cglm.h"

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

    u32 width = 1920;
    u32 height = 1080;

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    f32 boundsHalfSize = 40.0f;
    wr_aabb3 bounds = {
        .minX = -boundsHalfSize,
        .maxX = boundsHalfSize,
        .minY = -boundsHalfSize,
        .maxY = boundsHalfSize,
        .minZ = -boundsHalfSize,
        .maxZ = boundsHalfSize,
    };
    wr_boids boids;
    u32 numBoids = 20;
    wr_boids_init(&boids, &bounds, numBoids, 0.01f);
    wr_camera_init(&g_camera, 0.0f, 0.0f, 50.0f);

    wr_shdr_box boxShader = wr_shdr_box_init();
    wr_shdr_box_data boxShaderData = {0};

    wr_shdr_boids boidsShader = wr_shdr_boids_init();
    wr_shdr_boids_data boidsShaderData = {
        .color = { 0.5f, 0.8f, 0.2f },
        .light = {
            .position = {3.2f, 3.0f, 3.0f},
            .ambient  = {0.2f, 0.2f, 0.2f},
            .diffuse  = {0.5f, 0.5f, 0.5f},
            .specular = {1.0f, 1.0f, 1.0f},
        },
        .material = {
            .ambient = {1.0f, 0.5f, 0.31f},
            .diffuse = {1.0f, 0.5f, 0.31f},
            .specular = {0.5f, 0.5f, 0.5f},
            .shininess = 32.0f
        }
    };

    wr_shdr_light lightShader = wr_shdr_light_init();
    wr_shdr_light_data lightShaderData = {
        .color = {1.0f, 1.0f, 1.0f}
    };

    vec3 cubeVertices[] = {
        {-1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f, 1.0f,-1.0f},
        { 1.0f, 1.0f,-1.0f},
        {-1.0f, 1.0f,-1.0f},
        {-1.0f,-1.0f,-1.0f},

        {-1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f, 1.0f},
        { 1.0f, 1.0f, 1.0f},
        { 1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        {-1.0f,-1.0f, 1.0f},

        {-1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f,-1.0f},
        {-1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},

        { 1.0f, 1.0f, 1.0f},
        { 1.0f, 1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f, 1.0f},
        { 1.0f, 1.0f, 1.0f},

        {-1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f,-1.0f},

        {-1.0f, 1.0f,-1.0f},
        { 1.0f, 1.0f,-1.0f},
        { 1.0f, 1.0f, 1.0f},
        { 1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f,-1.0f},
    };

    vec3 boidVertices[] = {
        { 0.0f, 3.0f, 0.0f},
        {-1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f, 1.0f},
        { 0.0f, 3.0f, 0.0f},
        { 1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 0.0f, 3.0f, 0.0f},
        { 1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f,-1.0f},
        { 0.0f, 3.0f, 0.0f},
        {-1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
        {-1.0f,-1.0f, 1.0f},
        { 1.0f,-1.0f,-1.0f},
        { 1.0f,-1.0f, 1.0f},
    };

    const u32 normalsCount = ARRAY_SIZE(boidVertices);
    vec3 pyramidNormals[ARRAY_SIZE(boidVertices)];
    CalculateNormals(boidVertices, ARRAY_SIZE(boidVertices), pyramidNormals, normalsCount);

    GLuint cubeVao;
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    GLuint cubeVertexBuffer;
    glGenBuffers(1, &cubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint pyramidVao;
    glGenVertexArrays(1, &pyramidVao);
    glBindVertexArray(pyramidVao);

    GLuint pyramidVertexBuffer;
    glGenBuffers(1, &pyramidVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boidVertices), boidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint pyramidNormalsBuffer;
    glGenBuffers(1, &pyramidNormalsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidNormalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(1);

    GLuint boidModelsBuffer;
    glGenBuffers(1, &boidModelsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boidModelsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * numBoids, &boids.models[0], GL_STREAM_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(1 * sizeof(vec4)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(2 * sizeof(vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(3 * sizeof(vec4)));
    glEnableVertexAttribArray(5);

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

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

        // TODO: Delete this. Just for debugging
        if (dt < 16.667f)
        {
            SDL_Delay((u32)(16.667f - dt));
        }
        dt = 16.667f;

        stopping = SdlProcessEvents(dt);

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        wr_boids_update(&boids, dt);

        glUseProgram(boidsShader.program);
        glBindVertexArray(pyramidVao);
        glBindBuffer(GL_ARRAY_BUFFER, boidModelsBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mat4)* numBoids, &boids.models[0], GL_STREAM_DRAW);

        wr_camera_view(&g_camera, boidsShaderData.view);
        glm_vec3_copy(g_camera.position, boidsShaderData.camPos);

        glm_perspective(glm_rad(80.0f), (f32)width / (f32)height, 5.0f, 200.0f, boidsShaderData.proj);

        f32 radius = 5.0f;
        boidsShaderData.light.position[0] = sinf((f32)SDL_GetTicks() / 5000.0f) * radius;
        boidsShaderData.light.position[2] = cosf((f32)SDL_GetTicks() / 5000.0f) * radius;

        wr_shdr_boids_update_uniforms(boidsShader, boidsShaderData);

        glUseProgram(boidsShader.program);
        glBindVertexArray(pyramidVao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 18, numBoids);

        glm_mat4_identity(lightShaderData.model);
        glm_translate(lightShaderData.model, boidsShaderData.light.position);
        glm_mat4_copy(boidsShaderData.view, lightShaderData.view);
        glm_mat4_copy(boidsShaderData.proj, lightShaderData.proj);

        wr_shdr_light_update_uniforms(lightShader, lightShaderData);

        glUseProgram(lightShader.program);
        glBindVertexArray(lightVao);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        glm_mat4_identity(boxShaderData.model);
        vec3 scale = {boundsHalfSize, boundsHalfSize, boundsHalfSize};
        glm_scale(boxShaderData.model, scale);
        glm_mat4_copy(boidsShaderData.view, boxShaderData.view);
        glm_mat4_copy(boidsShaderData.proj, boxShaderData.proj);

        wr_shdr_box_update_uniforms(boxShader, boxShaderData);

        glUseProgram(boxShader.program);
        glBindVertexArray(cubeVao);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

