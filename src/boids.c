#include "boids.h"

#include "SDL_assert.h"
#include "aabb.h"

#include "cglm/cglm.h"

#include "stdlib.h"
#include "time.h"

#define MIN_VELOCITY 0.001f
#define MAX_VELOCITY 0.002f

static void vec3_rand(vec3 v)
{
    SDL_assert(v);

    f32 x = (f32)rand();
    f32 y = (f32)rand();
    f32 z = (f32)rand();

    v[0] = x;
    v[1] = y;
    v[2] = z;
}

static void vec3_rand_bounded(vec3 v, wr_aabb3 *bounds)
{
    SDL_assert(v);
    SDL_assert(bounds);

    f32 x = (f32)(rand() % (int)(bounds->maxX - bounds->minX));
    f32 y = (f32)(rand() % (int)(bounds->maxY - bounds->minY));
    f32 z = (f32)(rand() % (int)(bounds->maxZ - bounds->minZ));

    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void wr_boids_init(wr_boids *b, wr_aabb3 *bounds, u32 numBoids)
{
    SDL_assert(b);
    SDL_assert(numBoids <= WR_MAX_NUM_BOIDS);

    time_t t;
    srand((unsigned)time(&t));

    b->numBoids = numBoids;
    wr_aabb3_copy(bounds, &b->bounds);
    for (u32 i = 0; i < numBoids; i++)
    {
        vec3_rand_bounded(b->positions[i], &b->bounds);
        if (b->positions[i][0] > bounds->maxX)
            b->positions[i][0] = bounds->minX;
        if (b->positions[i][0] < bounds->minX)
            b->positions[i][0] = bounds->maxX;
        if (b->positions[i][1] > bounds->maxY)
            b->positions[i][1] = bounds->minY;
        if (b->positions[i][1] < bounds->minY)
            b->positions[i][1] = bounds->maxY;
        if (b->positions[i][2] > bounds->maxZ)
            b->positions[i][2] = bounds->minZ;
        if (b->positions[i][2] < bounds->minZ)
            b->positions[i][2] = bounds->maxZ;

        vec3_rand(b->velocities[i]);
        glm_vec3_clamp(b->velocities[i], MIN_VELOCITY, MAX_VELOCITY);
    }
}

void wr_boids_update(wr_boids* b, f32 dt)
{
    SDL_assert(b);

    u32 numBoids = b->numBoids;
    wr_aabb3* bounds = &b->bounds;
    for (u32 i = 0; i < numBoids; i++)
    {
        vec3 velocity;
        glm_vec3_scale(b->velocities[i], dt, velocity);

        glm_vec3_add(b->positions[i], velocity, b->positions[i]);

        if (b->positions[i][0] > bounds->maxX)
            b->positions[i][0] = bounds->minX;
        if (b->positions[i][0] < bounds->minX)
            b->positions[i][0] = bounds->maxX;
        if (b->positions[i][1] > bounds->maxY)
            b->positions[i][1] = bounds->minY;
        if (b->positions[i][1] < bounds->minY)
            b->positions[i][1] = bounds->maxY;
        if (b->positions[i][2] > bounds->maxZ)
            b->positions[i][2] = bounds->minZ;
        if (b->positions[i][2] < bounds->minZ)
            b->positions[i][2] = bounds->maxZ;
    }
}
