#include "SDL.h"
// #include "SDL_opengl.h"
// #include "SDL_opengl_glext.h"
// Keep these here just to lookup values

#define GLAPI extern
#define GLAPIENTRY __stdcall

typedef unsigned int    GLenum;
typedef unsigned char   GLboolean;
typedef unsigned int    GLbitfield;
typedef void            GLvoid;
typedef signed char     GLbyte;         /* 1-byte signed */
typedef short           GLshort;        /* 2-byte signed */
typedef int             GLint;          /* 4-byte signed */
typedef unsigned char   GLubyte;        /* 1-byte unsigned */
typedef unsigned short  GLushort;       /* 2-byte unsigned */
typedef unsigned int    GLuint;         /* 4-byte unsigned */
typedef int             GLsizei;        /* 4-byte signed */
typedef float           GLfloat;        /* single precision float */
typedef float           GLclampf;       /* single precision float in [0,1] */
typedef double          GLdouble;       /* double precision float */
typedef double          GLclampd;       /* double precision float in [0,1] */
typedef char            GLchar;

#ifdef __MACOSX__
typedef long GLsizeiptr;
typedef long GLintptr;
#else
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#endif

#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define WR_GL_LIST \
    /*  ret, name, params */ \
    GLE(void,   ClearColor,        GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) \
    GLE(void,   Clear,             GLbitfield mask) \
    GLE(void,   GenBuffers,        GLsizei n, GLuint *buffers) \
    GLE(void,   BindBuffer,        GLenum target, GLuint buffer) \
    GLE(void,   BufferData,        GLenum target, GLsizeiptr size, const void *data, GLenum usage) \
    GLE(GLuint, CreateShader,      GLenum type) \
    GLE(void,   DeleteShader,      GLenum shader) \
    GLE(void,   ShaderSource,      GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) \
    GLE(void,   AttachShader,      GLuint program, GLuint shader) \
    GLE(void,   GetShaderiv,       GLuint shader, GLenum pname, GLint *params) \
    GLE(void,   GetShaderInfoLog,  GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(GLuint, CreateProgram,     void) \
    GLE(void,   LinkProgram,       GLuint program) \
    GLE(void,   UseProgram,        GLuint program) \
    GLE(void,   GetProgramiv,      GLuint program, GLenum pname, GLint *params) \
    GLE(void,   GetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
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
