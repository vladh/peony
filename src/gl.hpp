#ifndef GL_H
#define GL_H

#include <stdint.h>

#include <glm/glm.hpp>


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;
typedef float real32;
typedef double real64;

#define global_variable static
#define local_persist static
#define internal static

typedef struct State {
  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  real32 test_vertices[512];
  real32 test_indices[512];
  glm::vec3 cube_positions[32];

  uint32 shader_program;
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
} State;

#endif
