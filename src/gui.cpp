namespace gui {
  internal void request_cursor(GuiState *gui_state, GLFWcursor *cursor) {
    gui_state->requested_cursor = cursor;
  }


  internal void set_cursor(GuiState *gui_state) {
    input::set_cursor(gui_state->input_state, gui_state->requested_cursor);
    gui_state->requested_cursor = nullptr;
  }


  internal void push_vertices(GuiState *gui_state, real32 *vertices, uint32 n_vertices) {
    // VAO/VBO must have been bound by start_drawing()
    glBufferSubData(
      GL_ARRAY_BUFFER,
      VERTEX_SIZE * gui_state->n_vertices_pushed,
      VERTEX_SIZE * n_vertices,
      vertices
    );

    gui_state->n_vertices_pushed += n_vertices;
  }


  internal v2 get_text_dimensions(
    FontAsset *font_asset, const char *str
  ) {
    // NOTE: This returns the dimensions around the main body of the text.
    // This does not include descenders.
    real32 line_height = fonts::font_unit_to_px(font_asset->height);
    real32 line_spacing = line_height * LINE_SPACING_FACTOR;
    real32 ascender = fonts::font_unit_to_px(font_asset->ascender);

    real32 start_x = 0.0f;
    real32 start_y = 0.0f - (line_height - ascender);
    real32 max_x = 0.0f;
    real32 curr_x = start_x;
    real32 curr_y = start_y;
    size_t str_length = strlen(str);

    for (uint32 idx = 0; idx < str_length; idx++) {
      char c = str[idx];

      if (c < 32) {
        if (c == '\n') {
          max_x = max(max_x, curr_x);
          curr_x = 0.0f;
          curr_y += line_spacing;
        }
        continue;
      }

      Character *character = font_asset->characters[c];

      if (!character) {
        logs::warning("Could not get character: %c", c);
        continue;
      }

      curr_x += fonts::frac_px_to_px(character->advance.x);
      curr_y += fonts::frac_px_to_px(character->advance.y);
    }

    max_x = max(max_x, curr_x);
    curr_y += line_height;

    return v2(max_x, curr_y);
  }


  internal v2 center_bb(
    v2 container_position,
    v2 container_dimensions,
    v2 element_dimensions
  ) {
    return ceil(
      container_position + (container_dimensions / 2.0f) - (element_dimensions / 2.0f)
    );
  }


  internal v2 add_element_to_container(
    GuiContainer *container, v2 element_dimensions
  ) {
    // When adding a new element, we need to ensure we have enough space.
    //
    // We need:
    // * Enough space for the element itself.
    // * If this is not the first element, enough space for one
    //   `element_margin` on the main axis.
    //
    // On the main axis, we will allocate new space of this size.
    // On the orthogonal axis, we will ensure the element's dimensions are at
    // least this big, taking the container padding into account/
    //
    // For example, if we add a button that is 200x20 to a container which already
    // has buttons, we will add (20 + element_margin) to its height, and ensure
    // its width is at least (200 + padding).

    v2 new_element_position = container->next_element_position;
    v2 orthogonal_direction = v2(
      container->direction.y, container->direction.x
    );

    v2 required_space = element_dimensions;
    if (container->n_elements > 0) {
      required_space += (container->element_margin * container->direction);
    }

    container->content_dimensions = (
      (container->content_dimensions + required_space) * container->direction
    ) + (
      max(container->content_dimensions, required_space) * orthogonal_direction
    );
    container->dimensions = container->content_dimensions +
      (container->padding * 2.0f) +
      v2(0.0f, container->title_bar_height);

    container->next_element_position = container->position +
      (
        (container->dimensions - container->padding + container->element_margin) *
        container->direction
      ) +
      (
        (container->padding + v2(0.0f, container->title_bar_height)) *
        orthogonal_direction
      );

    container->n_elements++;

    return new_element_position;
  }


  internal void draw_rect(
    GuiState *gui_state,
    v2 position,
    v2 dimensions,
    v4 color
  ) {
    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    real32 x0 = position.x;
    real32 x1 = x0 + dimensions.x;
    real32 y0 = (real32)gui_state->window_dimensions.y - position.y;
    real32 y1 = y0 - dimensions.y;

    real32 vertices[VERTEX_LENGTH * 6] = {
      x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x0, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x1, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a
    };
    push_vertices(gui_state, vertices, 6);
  }


  internal void draw_text(
    GuiState *gui_state,
    const char* font_name, const char *str,
    v2 position,
    v4 color
  ) {
    FontAsset *font_asset = fonts::get_by_name(&gui_state->font_assets, font_name);

    real32 line_height = fonts::font_unit_to_px(font_asset->height);
    real32 line_spacing = line_height * LINE_SPACING_FACTOR;
    real32 ascender = fonts::font_unit_to_px(font_asset->ascender);

    // NOTE: When changing this code, remember that the text positioning logic
    // needs to be replicated in `get_text_dimensions()`!
    real32 start_x = position.x;
    real32 start_y = position.y - (line_height - ascender);
    real32 curr_x = start_x;
    real32 curr_y = start_y;
    size_t str_length = strlen(str);

    for (uint32 idx = 0; idx < str_length; idx++) {
      char c = str[idx];

      if (c < 32) {
        if (c == '\n') {
          curr_x = start_x;
          curr_y += line_spacing;
        }
        continue;
      }

      Character *character = font_asset->characters[c];

      if (!character) {
        logs::warning("Could not get character: %c", c);
        continue;
      }

      real32 char_x = curr_x + character->bearing.x;
      real32 char_y = curr_y + (
        fonts::font_unit_to_px(font_asset->height) - character->bearing.y
      );

      real32 tex_x = (real32)character->tex_coords.x / gui_state->texture_atlas.size.x;
      real32 tex_y = (real32)character->tex_coords.y / gui_state->texture_atlas.size.y;
      real32 tex_w = (real32)character->size.x / gui_state->texture_atlas.size.x;
      real32 tex_h = (real32)character->size.y / gui_state->texture_atlas.size.y;

      real32 w = (real32)character->size.x;
      real32 h = (real32)character->size.y;

      curr_x += fonts::frac_px_to_px(character->advance.x);
      curr_y += fonts::frac_px_to_px(character->advance.y);

      // Skip glyphs with no pixels, like spaces.
      if (w <= 0 || h <= 0) {
        continue;
      }

      // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
      // Flip the y axis before drawing.
      real32 x0 = char_x;
      real32 x1 = x0 + w;
      real32 y0 = (real32)gui_state->window_dimensions.y - char_y;
      real32 y1 = y0 - h;

      real32 tex_x0 = (real32)tex_x;
      real32 tex_x1 = tex_x0 + tex_w;
      real32 tex_y0 = (real32)tex_y;
      real32 tex_y1 = tex_y0 + tex_h;

      real32 vertices[VERTEX_LENGTH * 6] = {
        x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
        x0, y1, tex_x0, tex_y1, color.r, color.g, color.b, color.a,
        x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
        x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
        x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
        x1, y0, tex_x1, tex_y0, color.r, color.g, color.b, color.a
      };
      push_vertices(gui_state, vertices, 6);
    }
  }


  internal void draw_text_shadow(
    GuiState *gui_state,
    const char* font_name, const char *str,
    v2 position,
    v4 color
  ) {
    draw_text(
      gui_state,
      font_name, str, position + TEXT_SHADOW_OFFSET,
      v4(0.0f, 0.0f, 0.0f, color.a * 0.2f)
    );
  }


  internal void draw_container(
    GuiState *gui_state,
    GuiContainer *container
  ) {
    draw_rect(
      gui_state,
      container->position,
      v2(
        container->dimensions.x, container->title_bar_height
      ),
      MAIN_DARKEN_COLOR
    );

    v2 text_dimensions = get_text_dimensions(
      fonts::get_by_name(&gui_state->font_assets, "body"),
      container->title
    );
    v2 centered_text_position = center_bb(
      container->position,
      v2(container->dimensions.x, container->title_bar_height),
      text_dimensions
    );
    v2 text_position = v2(
      container->position.x + container->padding.x,
      centered_text_position.y
    );
    draw_text_shadow(
      gui_state,
      "body",
      container->title,
      text_position,
      LIGHT_TEXT_COLOR
    );
    draw_text(
      gui_state,
      "body",
      container->title,
      text_position,
      LIGHT_TEXT_COLOR
    );

    draw_rect(
      gui_state,
      container->position + v2(0.0, container->title_bar_height),
      container->dimensions - v2(0.0, container->title_bar_height),
      WINDOW_BG_COLOR
    );
  }


  internal void draw_line(
    GuiState *gui_state,
    v2 start, v2 end,
    real32 thickness, v4 color
  ) {
    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    v2 delta = normalize(end - start) * thickness;

    //    ----------->
    // 0------------------3
    // |                  |
    // 1------------------2
    real32 x0 = start.x + delta.y;
    real32 y0 = gui_state->window_dimensions.y - start.y;
    real32 x1 = start.x;
    real32 y1 = gui_state->window_dimensions.y - start.y - delta.x;
    real32 x2 = end.x;
    real32 y2 = gui_state->window_dimensions.y - end.y - delta.x;
    real32 x3 = end.x + delta.y;
    real32 y3 = gui_state->window_dimensions.y - end.y;

    real32 vertices[VERTEX_LENGTH * 6] = {
      x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
      x3, y3, -1.0f, -1.0f, color.r, color.g, color.b, color.a
    };
    push_vertices(gui_state, vertices, 6);
  }


  internal void draw_frame(
    GuiState *gui_state,
    v2 position, v2 bottomright,
    v2 thickness, v4 color
  ) {
    draw_line(
      gui_state,
      v2(position.x, position.y),
      v2(bottomright.x, position.y),
      thickness.y,
      color
    );
    draw_line(
      gui_state,
      v2(position.x, bottomright.y - thickness.y),
      v2(bottomright.x, bottomright.y - thickness.y),
      thickness.y,
      color
    );
    draw_line(
      gui_state,
      v2(position.x, position.y),
      v2(position.x, bottomright.y),
      thickness.x,
      color
    );
    draw_line(
      gui_state,
      v2(bottomright.x - thickness.x, position.y),
      v2(bottomright.x - thickness.x, bottomright.y),
      thickness.x,
      color
    );
  }
}


void gui::update_screen_dimensions(
  GuiState *gui_state,
  uint32 new_window_width, uint32 new_window_height
) {
  gui_state->window_dimensions = v2(new_window_width, new_window_height);
}


void gui::update_mouse_button(GuiState *gui_state) {
  if (input::is_mouse_button_now_up(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
    gui_state->container_being_moved = nullptr;
  }
}


void gui::update_mouse(GuiState *gui_state) {
  if (gui_state->container_being_moved) {
    gui_state->container_being_moved->position += gui_state->input_state->mouse_offset;
  }
}


void gui::start_drawing(GuiState *gui_state) {
  glBindVertexArray(gui_state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, gui_state->vbo);
}


void gui::render(GuiState *gui_state) {
  glUseProgram(gui_state->shader_asset.program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gui_state->texture_atlas.texture_name);

  if (!gui_state->shader_asset.did_set_texture_uniforms) {
    shaders::set_int(&gui_state->shader_asset, "atlas_texture", 0);
    gui_state->shader_asset.did_set_texture_uniforms = true;
  }

  glDrawArrays(GL_TRIANGLES, 0, gui_state->n_vertices_pushed);
  gui_state->n_vertices_pushed = 0;

  set_cursor(gui_state);
}


GuiContainer* gui::make_container(
  GuiState *gui_state, const char *title, v2 position
) {
  GuiContainer *container = nullptr;
  for_each (container_candidate, gui_state->containers) {
    if (strcmp(container_candidate->title, title) == 0) {
      container = container_candidate;
      break;
    }
  }

  if (container) {
    // Check if we need to set this container as being moved.
    if (
      input::is_mouse_in_bb(
        gui_state->input_state,
        container->position,
        container->position + v2(
          container->dimensions.x, container->title_bar_height
        )
      ) &&
      input::is_mouse_button_now_down(
        gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT
      )
    ) {
      gui_state->container_being_moved = container;
    }

    // Draw the container with the information from the previous frame
    // if there is anything in it.
    if (container->content_dimensions != v2(0.0f, 0.0f)) {
      draw_container(gui_state, container);
    }
  } else {
    container = gui_state->containers.push();
    container->title = title;
    container->position = position;
    container->direction = v2(0.0f, 1.0f);
    container->padding = v2(20.0f);
    container->title_bar_height = 40.0f;
    container->element_margin = 20.0f;
  }

  // In all cases, clear this container.
  container->n_elements = 0;
  container->dimensions = container->padding * 2.0f;
  container->content_dimensions = v2(0.0f, 0.0f);
  container->next_element_position = container->position +
    container->padding +
    v2(0.0f, container->title_bar_height);

  return container;
}


void gui::draw_heading(
  GuiState *gui_state,
  const char *str,
  v4 color
) {
  v2 position = v2(
    center_bb(
      v2(0.0f, 0.0f),
      gui_state->window_dimensions,
      get_text_dimensions(
        fonts::get_by_name(&gui_state->font_assets, "heading"),
        str
      )
    ).x,
    90.0f
  );
  draw_text_shadow(gui_state, "heading", str, position, color);
  draw_text(gui_state, "heading", str, position, color);
}


bool32 gui::draw_toggle(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text,
  bool32 *toggle_state
) {
  bool32 is_pressed = false;

  v2 text_dimensions = get_text_dimensions(
    fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  v2 button_dimensions = TOGGLE_BUTTON_SIZE +
    BUTTON_DEFAULT_BORDER * 2.0f;
  v2 dimensions = v2(
    button_dimensions.x + TOGGLE_SPACING + text_dimensions.x,
    max(button_dimensions.y, text_dimensions.y)
  );

  v2 position = add_element_to_container(container, dimensions);

  v2 button_bottomright = position + button_dimensions;
  v2 text_centered_position = center_bb(
    position, button_dimensions, text_dimensions
  );
  v2 text_position = v2(
    position.x + button_dimensions.x + TOGGLE_SPACING,
    text_centered_position.y
  );

  v4 button_color;
  if (*toggle_state) {
    button_color = MAIN_COLOR;
  } else {
    button_color = LIGHT_COLOR;
  }

  if (input::is_mouse_in_bb(gui_state->input_state, position, button_bottomright)) {
    request_cursor(gui_state, gui_state->input_state->hand_cursor);
    if (*toggle_state) {
      button_color = MAIN_HOVER_COLOR;
    } else {
      button_color = LIGHT_HOVER_COLOR;
    }

    if (input::is_mouse_button_now_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (input::is_mouse_button_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      if (*toggle_state) {
        button_color = MAIN_ACTIVE_COLOR;
      } else {
        button_color = LIGHT_ACTIVE_COLOR;
      }
    }
  }

  draw_frame(
    gui_state,
    position,
    button_bottomright,
    TOGGLE_BUTTON_DEFAULT_BORDER,
    LIGHT_DARKEN_COLOR
  );
  draw_rect(
    gui_state,
    position + TOGGLE_BUTTON_DEFAULT_BORDER,
    button_dimensions - (TOGGLE_BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    gui_state,
    "body", text,
    text_position,
    LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


void gui::draw_named_value(
  GuiState *gui_state,
  GuiContainer *container,
  const char *name_text,
  const char *value_text
) {
  v2 name_text_dimensions = get_text_dimensions(
    fonts::get_by_name(&gui_state->font_assets, "body-bold"),
    name_text
  );
  v2 value_text_dimensions = get_text_dimensions(
    fonts::get_by_name(&gui_state->font_assets, "body"),
    value_text
  );
  // Sometimes we draw a value which is a rapidly changing number.
  // We don't want to container to wobble in size back and forth, so we round
  // the size of the value text to the next multiple of 50.
  value_text_dimensions.x = util::round_to_nearest_multiple(
    value_text_dimensions.x, 50.0f
  );
  v2 dimensions = v2(
    value_text_dimensions.x + NAMED_VALUE_NAME_WIDTH,
    max(name_text_dimensions.y, value_text_dimensions.y)
  );

  v2 position = add_element_to_container(container, dimensions);

  draw_text(
    gui_state,
    "body-bold", name_text,
    position,
    LIGHT_TEXT_COLOR
  );

  v2 value_text_position = position +
    v2(NAMED_VALUE_NAME_WIDTH, 0.0f);
  draw_text(
    gui_state,
    "body", value_text,
    value_text_position,
    LIGHT_TEXT_COLOR
  );
}


void gui::draw_body_text(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text
) {
  v2 dimensions = get_text_dimensions(
    fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  v2 position = add_element_to_container(container, dimensions);
  draw_text(gui_state, "body", text, position, LIGHT_TEXT_COLOR);
}


bool32 gui::draw_button(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text
) {
  bool32 is_pressed = false;

  v2 text_dimensions = get_text_dimensions(
    fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  v2 button_dimensions = text_dimensions +
    BUTTON_AUTOSIZE_PADDING +
    BUTTON_DEFAULT_BORDER * 2.0f;

  v2 position = add_element_to_container(container, button_dimensions);

  v2 bottomright = position + button_dimensions;
  v2 text_position = center_bb(position, button_dimensions, text_dimensions);

  v4 button_color = MAIN_COLOR;

  if (input::is_mouse_in_bb(gui_state->input_state, position, bottomright)) {
    request_cursor(gui_state, gui_state->input_state->hand_cursor);
    button_color = MAIN_HOVER_COLOR;

    if (input::is_mouse_button_now_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (input::is_mouse_button_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      button_color = MAIN_ACTIVE_COLOR;
    }
  }

  draw_frame(
    gui_state,
    position,
    bottomright,
    BUTTON_DEFAULT_BORDER,
    MAIN_DARKEN_COLOR
  );
  draw_rect(
    gui_state,
    position + BUTTON_DEFAULT_BORDER,
    button_dimensions - (BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    gui_state,
    "body", text,
    text_position,
    LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


void gui::draw_console(
  GuiState *gui_state,
  char *console_input_text
) {
  if (!g_console->is_enabled) {
    return;
  }

  FontAsset *font_asset = fonts::get_by_name(&gui_state->font_assets, "body");
  real32 line_height = fonts::font_unit_to_px(font_asset->height);
  real32 line_spacing = floor(
    line_height * CONSOLE_LINE_SPACING_FACTOR
  );

  // Draw console log
  {
    v2 next_element_position = v2(
      CONSOLE_PADDING.x, MAX_CONSOLE_LOG_HEIGHT
    );

    draw_rect(
      gui_state,
      v2(0.0f, 0.0f),
      v2(
        gui_state->window_dimensions.x,
        MAX_CONSOLE_LOG_HEIGHT
      ),
      CONSOLE_BG_COLOR
    );

    uint32 idx_line = g_console->idx_log_start;
    while (idx_line != g_console->idx_log_end) {
      v2 text_dimensions = get_text_dimensions(
        font_asset, g_console->log[idx_line]
      );
      next_element_position.y -= text_dimensions.y + line_spacing;
      draw_text(
        gui_state,
        "body", g_console->log[idx_line],
        next_element_position,
        LIGHT_TEXT_COLOR
      );

      idx_line++;
      if (idx_line == MAX_N_CONSOLE_LINES) {
        idx_line = 0;
      }
    }
  }

  // Draw console input
  {
    real32 console_input_height = line_height + (2.0f * CONSOLE_PADDING.y);
    v2 console_input_position = v2(0.0f, MAX_CONSOLE_LOG_HEIGHT);

    draw_rect(
      gui_state,
      console_input_position,
      v2(
        gui_state->window_dimensions.x,
        console_input_height
      ),
      MAIN_DARKEN_COLOR
    );

    draw_text(
      gui_state,
      "body", console_input_text,
      console_input_position + CONSOLE_PADDING,
      LIGHT_TEXT_COLOR
    );
  }
}


void gui::log(const char *format, ...) {
  char text[MAX_CONSOLE_LINE_LENGTH];
  va_list vargs;
  va_start(vargs, format);
  vsnprintf(text, sizeof(text), format, vargs);
  va_end(vargs);

  // Fill array in back-to-front.
  if (g_console->idx_log_start == 0) {
    g_console->idx_log_start = MAX_N_CONSOLE_LINES - 1;
  } else {
    g_console->idx_log_start--;
  }
  if (g_console->idx_log_start == g_console->idx_log_end) {
    if (g_console->idx_log_end == 0) {
      g_console->idx_log_end = MAX_N_CONSOLE_LINES - 1;
    } else {
      g_console->idx_log_end--;
    }
  }
  strcpy(g_console->log[g_console->idx_log_start], text);
}


void gui::set_heading(
  GuiState *gui_state,
  const char *text, real32 opacity,
  real32 fadeout_duration, real32 fadeout_delay
) {
  gui_state->heading_text = text;
  gui_state->heading_opacity = opacity;
  gui_state->heading_fadeout_duration = fadeout_duration;
  gui_state->heading_fadeout_delay = fadeout_delay;
}


GuiState* gui::init(
  GuiState* gui_state,
  MemoryPool *memory_pool,
  InputState *input_state,
  uint32 window_width, uint32 window_height
) {
  MemoryPool temp_memory_pool = {};

  gui_state->font_assets = Array<FontAsset>(
    memory_pool, 8, "font_assets"
  );
  gui_state->containers = Array<GuiContainer>(
    memory_pool, 32, "gui_containers"
  );
  gui_state->input_state = input_state;
  gui_state->window_dimensions = v2(window_width, window_height);
  materials::init_texture_atlas(&gui_state->texture_atlas, iv2(2000, 2000));

  // Shaders
  {
     shaders::init_shader_asset(
      &gui_state->shader_asset,
      &temp_memory_pool,
      "gui_generic", ShaderType::standard,
      "gui_generic.vert", "gui_generic.frag", ""
    );
  }

  // Fonts
  {
    FT_Library ft_library;

    if (FT_Init_FreeType(&ft_library)) {
      logs::error("Could not init FreeType");
      return nullptr;
    }

    fonts::init_font_asset(
      gui_state->font_assets.push(),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "body", MAIN_FONT_REGULAR, 18
    );

    fonts::init_font_asset(
      gui_state->font_assets.push(),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "body-bold", MAIN_FONT_BOLD, 18
    );

    fonts::init_font_asset(
      gui_state->font_assets.push(),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "heading", MAIN_FONT_REGULAR, 42
    );

    fonts::init_font_asset(
      gui_state->font_assets.push(),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "title", MAIN_FONT_REGULAR, 64
    );

    FT_Done_FreeType(ft_library);
  }

  // VAO
  {
    glGenVertexArrays(1, &gui_state->vao);
    glGenBuffers(1, &gui_state->vbo);
    glBindVertexArray(gui_state->vao);
    glBindBuffer(GL_ARRAY_BUFFER, gui_state->vbo);
    glBufferData(
      GL_ARRAY_BUFFER, VERTEX_SIZE * MAX_N_VERTICES,
      NULL, GL_DYNAMIC_DRAW
    );

    uint32 location;

    // position (vec2)
    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(0)
    );

    // tex_coords (vec2)
    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(2 * sizeof(real32))
    );

    // color (vec4)
    location = 2;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(4 * sizeof(real32))
    );
  }

  memory::destroy_memory_pool(&temp_memory_pool);

  log("Hello world!");

  return gui_state;
}
