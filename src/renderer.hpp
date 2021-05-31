#pragma once

#include "types.hpp"
#include "memory.hpp"
#include "state.hpp"

namespace renderer {
  void resize_renderer_buffers(
    MemoryPool *memory_pool,
    Array<Material> *materials,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void update_drawing_options(State *state, GLFWwindow *window);
  void init(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height,
    State *state
  );
  void render(State *state);
}
