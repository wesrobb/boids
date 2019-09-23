#pragma once

#include "cglm/types.h"
#include "SDL_types.h"

typedef char               i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef float              f32;
typedef double             f64;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

typedef struct wr_material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} wr_material;

typedef struct wr_light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} wr_light;

typedef struct wr_camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    f32 yaw;
    f32 pitch;

    // Camera options
    f32 movementSpeed;
    f32 mouseSensitivity;
    f32 zoom;
} wr_camera;

typedef enum wr_camera_movement {
    WR_CAMERA_NONE = 0,
    WR_CAMERA_FORWARD = 1,
    WR_CAMERA_BACKWARD = 2,
    WR_CAMERA_RIGHT = 4,
    WR_CAMERA_LEFT = 8,
} wr_camera_movement;
