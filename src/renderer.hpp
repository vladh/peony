#pragma once

#include "types.hpp"
#include "memory.hpp"
#include "state.hpp"

namespace renderer {
  void update_drawing_options(State *state, GLFWwindow *window);
  void init(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height,
    State *state
  );
  WindowInfo init_window();
  void destroy_window();
  void render(State *state);
}
