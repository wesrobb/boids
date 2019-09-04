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

#define GL_FALSE 0
#define GL_TRUE 1

#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_FLOAT                          0x1406
#define GL_TRIANGLES                      0x0004
#define GL_VERSION                        0x1F02
#define GL_DEPTH_TEST                     0x0B71

#define WR_GL_LIST \
    /*  ret, name, params */ \
    GLE(void,      ClearColor,              GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) \
    GLE(void,      Clear,                   GLbitfield mask) \
    GLE(void,      GenBuffers,              GLsizei n, GLuint *buffers) \
    GLE(void,      BindBuffer,              GLenum target, GLuint buffer) \
    GLE(void,      BufferData,              GLenum target, GLsizeiptr size, const void *data, GLenum usage) \
    GLE(GLuint,    CreateShader,            GLenum type) \
    GLE(void,      DeleteShader,            GLenum shader) \
    GLE(void,      ShaderSource,            GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) \
    GLE(void,      CompileShader,           GLuint shader) \
    GLE(void,      AttachShader,            GLuint program, GLuint shader) \
    GLE(void,      GetShaderiv,             GLuint shader, GLenum pname, GLint *params) \
    GLE(void,      GetShaderInfoLog,        GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(GLuint,    CreateProgram,           void) \
    GLE(void,      LinkProgram,             GLuint program) \
    GLE(void,      UseProgram,              GLuint program) \
    GLE(void,      GetProgramiv,            GLuint program, GLenum pname, GLint *params) \
    GLE(void,      GetProgramInfoLog,       GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(void,      VertexAttribPointer,     GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) \
    GLE(void,      EnableVertexAttribArray, GLuint index) \
    GLE(void,      GenVertexArrays,         GLsizei n, GLuint *arrays) \
    GLE(void,      BindVertexArray,         GLuint array) \
    GLE(void,      DrawArrays,              GLenum mode, GLint first, GLsizei count) \
    GLE(GLubyte *, GetString,               GLenum name) \
    GLE(GLint,     GetUniformLocation,      GLuint program, const GLchar *name) \
    GLE(void,      Uniform1f,               GLint location, GLfloat v0) \
    GLE(void,      Uniform2f,               GLint location, GLfloat v0, GLfloat v1) \
    GLE(void,      Uniform3f,               GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
    GLE(void,      Uniform4f,               GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    GLE(void,      Uniform1i,               GLint location, GLint v0) \
    GLE(void,      Uniform2i,               GLint location, GLint v0, GLint v1) \
    GLE(void,      Uniform3i,               GLint location, GLint v0, GLint v1, GLint v2) \
    GLE(void,      Uniform4i,               GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
    GLE(void,      Uniform1fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform2fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform3fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform4fv,              GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      Uniform1iv,              GLint location, GLsizei count, const GLint *value) \
    GLE(void,      Uniform2iv,              GLint location, GLsizei count, const GLint *value) \
    GLE(void,      Uniform3iv,              GLint location, GLsizei count, const GLint *value) \
    GLE(void,      Uniform4iv,              GLint location, GLsizei count, const GLint *value) \
    GLE(void,      UniformMatrix2fv,        GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GLE(void,      UniformMatrix3fv,        GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GLE(void,      UniformMatrix4fv,        GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GLE(void,      Viewport,                GLint x, GLint y, GLsizei width, GLsizei height) \
    GLE(void,      Enable,                  GLenum cap) \
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
