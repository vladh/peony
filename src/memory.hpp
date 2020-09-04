#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include <glm/glm.hpp>

#include "types.hpp"
#include "asset.hpp"
#include "models.hpp"
#include "control.hpp"
#include "camera.hpp"


#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)

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

  Camera camera;

  Control control;

  bool32 is_wireframe_on;

  real64 t;
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
