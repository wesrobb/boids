#include "opengl.h"

#include "SDL.h"

void wr_opengl_init()
{
#define GLE(ret, name, ...) gl##name = (name##Fn *)SDL_GL_GetProcAddress("gl" #name);
    WR_GL_LIST
#undef GLE
}
