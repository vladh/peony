#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include "types.hpp"


struct Camera {
  real64 yaw;
  real64 pitch;
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  real32 speed;
  real32 fov;
  real32 near;
  real32 far;
  glm::mat4 view;
  glm::mat4 projection;
};

void camera_init(Camera *camera);
void camera_move_front_back(Camera *camera, real32 sign);
void camera_move_left_right(Camera *camera, real32 sign);
void camera_move_up_down(Camera *camera, real32 sign);
void camera_update_mouse(Camera *camera, glm::vec2 mouse_offset);
void camera_update_matrices(
  Camera *camera, real64 window_width, real64 window_height
);

#endif
