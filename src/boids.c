#include "boids.h"

#include "SDL_assert.h"
#include "aabb.h"

#include "cglm/cglm.h"

#include "stdlib.h"
#include "time.h"

#define MIN_VELOCITY 0.0005f
#define MAX_VELOCITY 0.004f

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

        vec3_rand(b->velocities[i], MIN_VELOCITY, MAX_VELOCITY);
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
