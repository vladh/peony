void DebugDraw::push_vertices(
  DebugDrawState *debug_draw_state,
  DebugDrawVertex vertices[],
  uint32 n_vertices
) {
  for_range (0, n_vertices) {
    debug_draw_state->vertices[debug_draw_state->n_vertices_pushed + idx] =
      vertices[idx];
  }
  debug_draw_state->n_vertices_pushed += n_vertices;
}


void DebugDraw::draw_line(
  DebugDrawState *debug_draw_state,
  glm::vec3 start_pos,
  glm::vec3 end_pos,
  glm::vec4 color
) {
  DebugDrawVertex vertices[2];
  vertices[0] = {start_pos, color};
  vertices[1] = {end_pos, color};
  push_vertices(debug_draw_state, vertices, 2);
}


void DebugDraw::draw_ray(
  DebugDrawState *debug_draw_state,
  Ray *ray,
  real32 length,
  glm::vec4 color
) {
  glm::vec3 end_pos = ray->origin + ray->direction * length;
  glm::vec3 x_axis = Util::get_orthogonal_vector(&ray->direction);
  glm::vec3 z_axis = glm::cross(ray->direction, x_axis);
  real32 chevron_size = 0.2f;
  glm::vec3 chevron_1_pos = end_pos + ((-ray->direction + x_axis) * chevron_size);
  glm::vec3 chevron_2_pos = end_pos + ((-ray->direction - x_axis) * chevron_size);
  glm::vec3 chevron_3_pos = end_pos + ((-ray->direction + z_axis) * chevron_size);
  glm::vec3 chevron_4_pos = end_pos + ((-ray->direction - z_axis) * chevron_size);
  draw_line(debug_draw_state, ray->origin, end_pos, color);
  draw_line(debug_draw_state, chevron_1_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_2_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_3_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_4_pos, end_pos, color);
}


void DebugDraw::draw_quad(
  DebugDrawState *debug_draw_state,
  glm::vec3 p1, // clockwise: top left
  glm::vec3 p2, // top right
  glm::vec3 p3, // bottom right
  glm::vec3 p4, // bottom left
  glm::vec4 color
) {
  draw_line(debug_draw_state, p1, p2, color);
  draw_line(debug_draw_state, p2, p3, color);
  draw_line(debug_draw_state, p3, p4, color);
  draw_line(debug_draw_state, p4, p1, color);
}


void DebugDraw::draw_box(
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
) {
  draw_quad(debug_draw_state, p1, p2, p3, p4, color);
  draw_quad(debug_draw_state, p5, p6, p7, p8, color);
  draw_quad(debug_draw_state, p1, p2, p6, p5, color);
  draw_quad(debug_draw_state, p2, p3, p7, p6, color);
  draw_quad(debug_draw_state, p3, p4, p8, p7, color);
  draw_quad(debug_draw_state, p4, p1, p5, p8, color);
}


void DebugDraw::draw_obb(
  DebugDrawState *debug_draw_state,
  Obb *obb,
  glm::vec4 color
) {
  glm::vec3 z_axis = glm::cross(obb->x_axis, obb->y_axis);
  glm::vec3 dir1 = obb->x_axis * obb->extents[0];
  glm::vec3 dir2 = obb->y_axis * obb->extents[1];
  glm::vec3 dir3 = z_axis * obb->extents[2];
  glm::vec3 p1 = obb->center - dir1 + dir2 - dir3;
  glm::vec3 p2 = obb->center + dir1 + dir2 - dir3;
  glm::vec3 p3 = obb->center + dir1 + dir2 + dir3;
  glm::vec3 p4 = obb->center - dir1 + dir2 + dir3;
  glm::vec3 p5 = obb->center - dir1 - dir2 - dir3;
  glm::vec3 p6 = obb->center + dir1 - dir2 - dir3;
  glm::vec3 p7 = obb->center + dir1 - dir2 + dir3;
  glm::vec3 p8 = obb->center - dir1 - dir2 + dir3;
  draw_quad(debug_draw_state, p1, p2, p3, p4, color);
  draw_quad(debug_draw_state, p5, p6, p7, p8, color);
  draw_quad(debug_draw_state, p1, p2, p6, p5, color);
  draw_quad(debug_draw_state, p2, p3, p7, p6, color);
  draw_quad(debug_draw_state, p3, p4, p8, p7, color);
  draw_quad(debug_draw_state, p4, p1, p5, p8, color);
}


void DebugDraw::render(DebugDrawState *debug_draw_state) {
  glBindVertexArray(debug_draw_state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, debug_draw_state->vbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    DEBUGDRAW_VERTEX_SIZE * debug_draw_state->n_vertices_pushed,
    debug_draw_state->vertices,
    GL_STATIC_DRAW
  );

  glUseProgram(debug_draw_state->shader_asset.program);

  glDrawArrays(GL_LINES, 0, debug_draw_state->n_vertices_pushed);
  debug_draw_state->n_vertices_pushed = 0;
}


DebugDrawState* DebugDraw::init_debug_draw_state(
  DebugDrawState* debug_draw_state,
  MemoryPool *memory_pool
) {
  MemoryPool temp_memory_pool = {};

  // Shaders
  {
     Shaders::init_shader_asset(
      &debug_draw_state->shader_asset,
      &temp_memory_pool,
      "debugdraw", ShaderType::standard,
      "debugdraw.vert", "debugdraw.frag", ""
    );
    debug_draw_state->shader_asset.did_set_texture_uniforms = true;
  }

  // VAO
  {
    glGenVertexArrays(1, &debug_draw_state->vao);
    glGenBuffers(1, &debug_draw_state->vbo);
    glBindVertexArray(debug_draw_state->vao);
    glBindBuffer(GL_ARRAY_BUFFER, debug_draw_state->vbo);
    glBufferData(
      GL_ARRAY_BUFFER, DEBUGDRAW_VERTEX_SIZE * DEBUGDRAW_MAX_N_VERTICES,
      NULL, GL_DYNAMIC_DRAW
    );

    uint32 location;

    // position (vec3)
    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 3, GL_FLOAT, GL_FALSE, DEBUGDRAW_VERTEX_SIZE, (void*)(0)
    );

    // color (vec4)
    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 4, GL_FLOAT, GL_FALSE, DEBUGDRAW_VERTEX_SIZE, (void*)(3 * sizeof(real32))
    );
  }

  Memory::destroy_memory_pool(&temp_memory_pool);

  return debug_draw_state;
}