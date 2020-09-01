#ifndef GL_H
#define GL_H

#include <stdint.h>

#include <glm/glm.hpp>

#include "types.hpp"
#include "models.hpp"


typedef struct State {
  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  real32 test_vertices[512];
  real32 test_indices[512];
  glm::vec3 cube_positions[32];

  uint32 alpaca_shader_program;
  uint32 backpack_shader_program;
  uint32 goose_shader_program;
  uint32 vao;
  uint32 test_texture;

  real64 yaw;
  real64 pitch;

  glm::vec3 camera_pos;
  glm::vec3 camera_front;
  glm::vec3 camera_up;
  real32 camera_speed;
  real32 camera_fov;
  real32 camera_near;
  real32 camera_far;

  bool32 mouse_has_moved;
  real64 mouse_last_x;
  real64 mouse_last_y;
  real64 mouse_sensitivity;

  bool32 is_wireframe_on;

  Model models[4];
  uint32 n_models;
} State;

#endif
