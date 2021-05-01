#ifndef DEBUGDRAW_HPP
#define DEBUGDRAW_HPP

constexpr uint32 DEBUGDRAW_MAX_N_VERTICES = 2048;
constexpr uint32 DEBUGDRAW_VERTEX_LENGTH = 7;
constexpr size_t DEBUGDRAW_VERTEX_SIZE = sizeof(real32) * DEBUGDRAW_VERTEX_LENGTH;

struct DebugDrawVertex {
  glm::vec3 position;
  glm::vec4 color;
};

struct DebugDrawState {
  ShaderAsset shader_asset;
  uint32 vao;
  uint32 vbo;
  DebugDrawVertex vertices[DEBUGDRAW_MAX_N_VERTICES];
  uint32 n_vertices_pushed;
};

namespace DebugDraw {
  void push_vertices(
    DebugDrawState *debug_draw_state,
    DebugDrawVertex vertices[],
    uint32 n_vertices
  );
  void draw_line(
    DebugDrawState *debug_draw_state,
    glm::vec3 start_pos,
    glm::vec3 end_pos,
    glm::vec4 color
  );
  void draw_quad(
    DebugDrawState *debug_draw_state,
    glm::vec3 p1, // clockwise: top left
    glm::vec3 p2, // top right
    glm::vec3 p3, // bottom right
    glm::vec3 p4, // bottom left
    glm::vec4 color
  );
  void draw_box(
    DebugDrawState *debug_draw_state,
    glm::vec3 p1, // clockwise top face: top left
    glm::vec3 p2, // top right
    glm::vec3 p3, // bottom right
    glm::vec3 p4, // top left
    glm::vec3 p5, // clockwise bottom face: top left
    glm::vec3 p6, // top right
    glm::vec3 p7, // bottom right
    glm::vec3 p8, // top left
    glm::vec4 color
  );
  void draw_obb(
    DebugDrawState *debug_draw_state,
    Obb *obb,
    glm::vec4 color
  );
  void render(DebugDrawState *debug_draw_state);
  DebugDrawState* init_debug_draw_state(
    DebugDrawState *debug_draw_state,
    MemoryPool *memory_pool
  );
}

#endif
