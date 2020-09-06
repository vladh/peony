#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include <glm/glm.hpp>

#include "types.hpp"
#include "asset.hpp"
#include "models.hpp"
#include "control.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "array.hpp"


struct State {
  Camera camera;
  Control control;

  real64 t;
  real64 dt;
  bool32 is_wireframe_on;
  bool32 is_cursor_disabled;

  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  uint32 test_vao;
  uint32 test_texture;
  real32 test_vertices[512];
  real32 test_indices[512];
  glm::vec3 test_cube_positions[32];

  Array<Entity> entities;
  Array<Entity*> found_entities;
  Array<ShaderAsset> shader_assets;
  Array<ModelAsset*> model_assets;

  bool32 key_states[1024];
  bool32 prev_key_states[1024];
};

struct MemoryPool {
  const char *name;
  uint8 *memory;
  uint32 size;
  uint32 used;
};

struct Memory {
  State *state_memory;
  uint32 state_memory_size;

  MemoryPool asset_memory_pool;
};

MemoryPool memory_make_memory_pool(const char *name, uint32 size);
void* memory_push_memory_to_pool(MemoryPool *pool, uint32 size);

#endif
