#pragma once

#include "types.h"

vec3 g_boidVertices[18];

void wr_boids_init(wr_boids *b, wr_aabb3 *bounds, u32 numBoids, f32 speed, vec3 color);

void wr_boids_update(wr_boids *b, f32 dt);
