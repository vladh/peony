#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include <glm/glm.hpp>

#include "types.hpp"
#include "models.hpp"


#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)

typedef struct Shader {
  uint32 program;
} Shader;

typedef struct AssetInfo {
  const char *name;
} AssetInfo;

typedef struct ModelAsset {
  AssetInfo info;
  Model model;
} ModelAsset;

typedef struct ShaderAsset {
  AssetInfo info;
  Shader shader;
} ShaderAsset;

typedef struct State {
  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  real32 test_vertices[512];
  real32 test_indices[512];
  glm::vec3 cube_positions[32];

  ShaderAsset *shader_assets;
  uint32 n_shader_assets;
  uint32 max_n_shader_assets;

  ModelAsset *model_assets[128];
  uint32 n_model_assets;

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
} State;

typedef struct MemoryPool {
  const char *name;
  uint8 *memory;
  uint32 size;
  uint32 used;
} MemoryPool;

typedef struct Memory {
  State *state_memory;
  uint32 state_memory_size;

  MemoryPool asset_memory_pool;
} Memory;

MemoryPool memory_make_memory_pool(const char *name, uint32 size);
void* memory_push_memory_to_pool(MemoryPool *pool, uint32 size);

#endif
