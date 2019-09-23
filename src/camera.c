#include "camera.h"

#include "cglm/cglm.h"
#include "SDL_assert.h"

// Default camera values
#define DEFAULT_CAMERA_YAW    -90.0f
#define DEFAULT_CAMERA_PITCH   0.0f
#define DEFAULT_CAMERA_ZOOM    45.0f
#define MOVEMENT_SPEED         0.5f
#define MOUSE_SENSITIVITY      0.1f

static void update(wr_camera *c)
{
    SDL_assert(c);

    vec3 front;

    f32 yaw = glm_rad(c->yaw);
    f32 pitch = glm_rad(c->pitch);

    front[0] = cosf(yaw)*cosf(pitch);
    front[1] = sinf(pitch);
    front[2] = sinf(yaw)*cosf(pitch);
    glm_vec3_normalize(front);

    vec3 temp;
    glm_vec3_cross(front, c->worldUp, temp);
    glm_vec3_normalize(temp);
    glm_vec3_copy(temp, c->right);

    glm_vec3_cross(c->right, front, temp);
    glm_vec3_normalize(temp);
    glm_vec3_copy(temp, c->up);

    glm_vec3_copy(front, c->front);
}

void wr_camera_init(wr_camera *c, f32 x, f32 y, f32 z)
{
    SDL_assert(c);

    c->pitch = DEFAULT_CAMERA_PITCH;
    c->yaw = DEFAULT_CAMERA_YAW;
    glm_vec3_copy((vec3){x, y, z}, c->position);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c->up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c->worldUp);

    c->movementSpeed = MOVEMENT_SPEED;
    c->mouseSensitivity = MOUSE_SENSITIVITY;
    c->zoom = DEFAULT_CAMERA_ZOOM;

    update(c);
}

void wr_camera_update_mouse(wr_camera *c, i32 xMouseRelative, i32 yMouseRelative)
{
    SDL_assert(c);

    c->yaw += MOUSE_SENSITIVITY * xMouseRelative;
    c->pitch -= MOUSE_SENSITIVITY * yMouseRelative;

    if (c->pitch > 89)
        c->pitch = 89;

    if (c->pitch < -89)
        c->pitch = -89;

    update(c);
}

void wr_camera_update_keyboard(wr_camera *c, wr_camera_movement movement, f32 dt)
{
    SDL_assert(c);

    f32 velocity = c->movementSpeed * dt;
    if (movement & WR_CAMERA_FORWARD)
        glm_vec3_muladds(c->front, velocity, c->position);
    if (movement & WR_CAMERA_BACKWARD)
        glm_vec3_mulsubs(c->front, velocity, c->position);
    if (movement & WR_CAMERA_RIGHT)
        glm_vec3_muladds(c->right, velocity, c->position);
    if (movement & WR_CAMERA_LEFT)
        glm_vec3_mulsubs(c->right, velocity, c->position);

    update(c);
}

void wr_camera_view(wr_camera *c, mat4 dest)
{
    SDL_assert(c);

    vec3 cameraFront;
    glm_vec3_add(c->position, c->front, cameraFront);
    glm_lookat(c->position,
               cameraFront,
               c->up,
               dest);
}
