#pragma once

#include "types.h"

void wr_boids_init(wr_boids *b, wr_aabb3 *bounds, u32 numBoids);

void wr_boids_update(wr_boids *b, f32 dt);
