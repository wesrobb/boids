#pragma once

#include "types.h"

void wr_camera_init(wr_camera* c, f32 x, f32 y, f32 z);

void wr_camera_update_mouse(wr_camera* c, i32 xMouseRelative, i32 yMouseRelative);

void wr_camera_update_keyboard(wr_camera* c, wr_camera_movement movement, f32 dt);

void wr_camera_view(wr_camera* c, mat4 dest);
