#include "SDL.h"
// #include "SDL_opengl.h"
// #include "SDL_opengl_glext.h"
// Keep these here just to lookup values

#define GLAPI extern
#define GLAPIENTRY __stdcall

typedef unsigned int	GLenum;
typedef unsigned char	GLboolean;
typedef unsigned int	GLbitfield;
typedef void		GLvoid;
typedef signed char	GLbyte;		/* 1-byte signed */
typedef short		GLshort;	/* 2-byte signed */
typedef int		GLint;		/* 4-byte signed */
typedef unsigned char	GLubyte;	/* 1-byte unsigned */
typedef unsigned short	GLushort;	/* 2-byte unsigned */
typedef unsigned int	GLuint;		/* 4-byte unsigned */
typedef int		GLsizei;	/* 4-byte signed */
typedef float		GLfloat;	/* single precision float */
typedef float		GLclampf;	/* single precision float in [0,1] */
typedef double		GLdouble;	/* double precision float */
typedef double		GLclampd;	/* double precision float in [0,1] */

#define GL_COLOR_BUFFER_BIT			0x00004000

#define WR_GL_LIST \
    /*  ret, name, params */ \
    GLE(void, ClearColor, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) \
    GLE(void, Clear,      GLbitfield mask) \
    /* end */

#define GLE(ret, name, ...) typedef ret GLAPIENTRY name##Fn(__VA_ARGS__); static name##Fn *gl##name;
WR_GL_LIST
#undef GLE

static void wr_opengl_init()
{
#define GLE(ret, name, ...) gl##name = (name##Fn *)SDL_GL_GetProcAddress("gl" #name);
    WR_GL_LIST
#undef GLE
}
