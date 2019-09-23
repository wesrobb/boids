#include "aabb.h"

#include "SDL_assert.h"

void wr_aabb3_copy(wr_aabb3* src, wr_aabb3* dest)
{
    SDL_assert(src);
    SDL_assert(dest);

    dest->minX = src->minX;
    dest->minY = src->minY;
    dest->minZ = src->minZ;
    dest->maxX = src->maxX;
    dest->maxY = src->maxY;
    dest->maxZ = src->maxZ;
}