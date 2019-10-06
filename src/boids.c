#include "boids.h"

#include "aabb.h"
#include "collision.h"

#include "SDL.h"
#include "cglm/cglm.h"

#include "stdlib.h"
#include "time.h"

#define MIN_VELOCITY 0.0005f
#define MAX_VELOCITY 0.004f


vec3 g_boidVertices[] = {
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

static f32 f32_rand_range(f32 min, f32 max)
{
    return min + (float)(rand()) / ((float)(RAND_MAX/(max-min)));
}


static void vec3_rand(vec3 v, f32 min, f32 max)
{
    SDL_assert(v != 0);

    f32 x = f32_rand_range(min, max);
    f32 y = f32_rand_range(min, max);
    f32 z = f32_rand_range(min, max);

    v[0] = x;
    v[1] = y;
    v[2] = z;
}

static void vec3_rand_bounded(vec3 v, wr_aabb3 *bounds)
{
    SDL_assert(v != 0);
    SDL_assert(bounds);

    f32 x = f32_rand_range(bounds->minX, bounds->maxX);
    f32 y = f32_rand_range(bounds->minY, bounds->maxY);
    f32 z = f32_rand_range(bounds->minZ, bounds->maxZ);

    v[0] = x;
    v[1] = y;
    v[2] = z;
}

static void updateBoid(vec3 dir, vec3 pos, mat4 model, wr_aabb3 *bounds)
{
    if (pos[0] > bounds->maxX)
        pos[0] = bounds->minX;
    if (pos[0] < bounds->minX)
        pos[0] = bounds->maxX;
    if (pos[1] > bounds->maxY)
        pos[1] = bounds->minY;
    if (pos[1] < bounds->minY)
        pos[1] = bounds->maxY;
    if (pos[2] > bounds->maxZ)
        pos[2] = bounds->minZ;
    if (pos[2] < bounds->minZ)
        pos[2] = bounds->maxZ;

    glm_vec3_normalize(dir);

    vec3 right;
    vec3 boidUp = {0.0f, 1.0f, 0.0f};
    glm_vec3_cross(dir, boidUp, right);
    glm_vec3_normalize(right);

    f32 angle = SDL_acosf(glm_vec3_dot(dir, boidUp));

    glm_mat4_identity(model);
    glm_translate(model, pos);
    glm_rotate(model, -angle, right);
}

void wr_boids_init(wr_boids *b, wr_aabb3 *bounds, u32 numBoids, f32 speed, vec3 color)
{
    SDL_assert(b);
    SDL_assert(numBoids <= WR_MAX_NUM_BOIDS);

    time_t t;
    srand((unsigned)time(&t));

    b->numBoids = numBoids;
    b->speed = speed;
    wr_aabb3_copy(bounds, &b->bounds);
    for (u32 i = 0; i < numBoids; i++)
    {
        vec3_rand_bounded(b->positions[i], &b->bounds);
        vec3_rand(b->directions[i], -1.0f, 1.0f);
        glm_vec3_copy(color, b->colors[i]);

        updateBoid(b->directions[i], b->positions[i], b->models[i], bounds);
    }
}

static void convert_boid_verts_to_world_positions(mat4 model, vec3 dest[18])
{
    for (u32 i = 0; i < 18; i++)
    {
        vec4 temp;
        glm_mat4_mulv3(model, g_boidVertices[i], 1.0f, temp);
        glm_vec4_copy3(temp, dest[i]);
    }
}

void wr_boids_update(wr_boids* b, f32 dt)
{
    SDL_assert(b);

    u32 numBoids = b->numBoids;
    f32 speed = b->speed * dt;
    wr_aabb3* bounds = &b->bounds;
    for (u32 i = 0; i < numBoids; i++)
    {
        vec3 velocity;
        glm_vec3_scale(b->directions[i], speed, velocity);
        glm_vec3_add(b->positions[i], velocity, b->positions[i]);

        updateBoid(b->directions[i], b->positions[i], b->models[i], bounds);
    }

    vec3 iWorldVerts[18];
    vec3 jWorldVerts[18];

    // All the boids are in their new positions. Now check collisions
    for (u32 i = 0; i < numBoids - 1; i++)
    {
        convert_boid_verts_to_world_positions(b->models[i], iWorldVerts);
        for (u32 j = i + 1; j < numBoids; j++)
        {
            convert_boid_verts_to_world_positions(b->models[j], jWorldVerts);
            if (gjk_collide(iWorldVerts[0], 18, jWorldVerts[0], 18))
            {
                glm_vec3_copy((vec3){1.0f, 0.0f, 0.0f}, b->colors[i]);
                glm_vec3_copy((vec3){1.0f, 0.0f, 0.0f}, b->colors[j]);
            }
        }
    }
}
