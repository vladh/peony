namespace debugdraw {
  internal void push_vertices(
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
}


void debugdraw::draw_line(
  DebugDrawState *debug_draw_state,
  v3 start_pos,
  v3 end_pos,
  v4 color
) {
  DebugDrawVertex vertices[2];
  vertices[0] = {start_pos, color};
  vertices[1] = {end_pos, color};
  push_vertices(debug_draw_state, vertices, 2);
}


void debugdraw::draw_ray(
  DebugDrawState *debug_draw_state,
  Ray *ray,
  real32 length,
  v4 color
) {
  v3 end_pos = ray->origin + ray->direction * length;
  v3 x_axis = util::get_orthogonal_vector(&ray->direction);
  v3 z_axis = cross(ray->direction, x_axis);
  real32 chevron_size = 0.2f;
  v3 chevron_1_pos = end_pos + ((-ray->direction + x_axis) * chevron_size);
  v3 chevron_2_pos = end_pos + ((-ray->direction - x_axis) * chevron_size);
  v3 chevron_3_pos = end_pos + ((-ray->direction + z_axis) * chevron_size);
  v3 chevron_4_pos = end_pos + ((-ray->direction - z_axis) * chevron_size);
  draw_line(debug_draw_state, ray->origin, end_pos, color);
  draw_line(debug_draw_state, chevron_1_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_2_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_3_pos, end_pos, color);
  draw_line(debug_draw_state, chevron_4_pos, end_pos, color);
}


void debugdraw::draw_quad(
  DebugDrawState *debug_draw_state,
  v3 p1, // clockwise: top left
  v3 p2, // top right
  v3 p3, // bottom right
  v3 p4, // bottom left
  v4 color
) {
  draw_line(debug_draw_state, p1, p2, color);
  draw_line(debug_draw_state, p2, p3, color);
  draw_line(debug_draw_state, p3, p4, color);
  draw_line(debug_draw_state, p4, p1, color);
}


void debugdraw::draw_box(
  DebugDrawState *debug_draw_state,
  v3 p1, // clockwise top face: top left
  v3 p2, // top right
  v3 p3, // bottom right
  v3 p4, // top left
  v3 p5, // clockwise bottom face: top left
  v3 p6, // top right
  v3 p7, // bottom right
  v3 p8, // top left
  v4 color
) {
  draw_quad(debug_draw_state, p1, p2, p3, p4, color);
  draw_quad(debug_draw_state, p5, p6, p7, p8, color);
  draw_quad(debug_draw_state, p1, p2, p6, p5, color);
  draw_quad(debug_draw_state, p2, p3, p7, p6, color);
  draw_quad(debug_draw_state, p3, p4, p8, p7, color);
  draw_quad(debug_draw_state, p4, p1, p5, p8, color);
}


void debugdraw::draw_obb(
  DebugDrawState *debug_draw_state,
  Obb *obb,
  v4 color
) {
  v3 z_axis = cross(obb->x_axis, obb->y_axis);
  v3 dir1 = obb->x_axis * obb->extents[0];
  v3 dir2 = obb->y_axis * obb->extents[1];
  v3 dir3 = z_axis * obb->extents[2];
  v3 p1 = obb->center - dir1 + dir2 - dir3;
  v3 p2 = obb->center + dir1 + dir2 - dir3;
  v3 p3 = obb->center + dir1 + dir2 + dir3;
  v3 p4 = obb->center - dir1 + dir2 + dir3;
  v3 p5 = obb->center - dir1 - dir2 - dir3;
  v3 p6 = obb->center + dir1 - dir2 - dir3;
  v3 p7 = obb->center + dir1 - dir2 + dir3;
  v3 p8 = obb->center - dir1 - dir2 + dir3;
  draw_quad(debug_draw_state, p1, p2, p3, p4, color);
  draw_quad(debug_draw_state, p5, p6, p7, p8, color);
  draw_quad(debug_draw_state, p1, p2, p6, p5, color);
  draw_quad(debug_draw_state, p2, p3, p7, p6, color);
  draw_quad(debug_draw_state, p3, p4, p8, p7, color);
  draw_quad(debug_draw_state, p4, p1, p5, p8, color);
}


void debugdraw::draw_point(
  DebugDrawState *debug_draw_state,
  v3 position,
  real32 size,
  v4 color
) {
  Obb obb = {
    .center=position,
    .x_axis=v3(1.0f, 0.0f, 0.0f),
    .y_axis=v3(0.0f, 1.0f, 0.0f),
    .extents=v3(size),
  };
  draw_obb(debug_draw_state, &obb, color);
}


void debugdraw::render(DebugDrawState *debug_draw_state) {
  glBindVertexArray(debug_draw_state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, debug_draw_state->vbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    VERTEX_SIZE * debug_draw_state->n_vertices_pushed,
    debug_draw_state->vertices,
    GL_STATIC_DRAW
  );

  glUseProgram(debug_draw_state->shader_asset.program);

  glDrawArrays(GL_LINES, 0, debug_draw_state->n_vertices_pushed);
  debug_draw_state->n_vertices_pushed = 0;
}


DebugDrawState* debugdraw::init(
  DebugDrawState* debug_draw_state,
  MemoryPool *memory_pool
) {
  MemoryPool temp_memory_pool = {};

  // Shaders
  {
     shaders::init_shader_asset(
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
      GL_ARRAY_BUFFER, VERTEX_SIZE * MAX_N_VERTICES,
      NULL, GL_DYNAMIC_DRAW
    );

    uint32 location;

    // position (vec3)
    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(0)
    );

    // color (vec4)
    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(3 * sizeof(real32))
    );
  }

  memory::destroy_memory_pool(&temp_memory_pool);

  return debug_draw_state;
}
