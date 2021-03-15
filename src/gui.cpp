void Gui::update_screen_dimensions(
  GuiState *gui_state,
  uint32 new_window_width, uint32 new_window_height
) {
  gui_state->window_dimensions = glm::vec2(new_window_width, new_window_height);
}


void Gui::update_mouse_button(GuiState *gui_state) {
  if (Input::is_mouse_button_now_up(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
    gui_state->container_being_moved = nullptr;
  }
}


void Gui::update_mouse(GuiState *gui_state) {
  if (gui_state->container_being_moved) {
    gui_state->container_being_moved->position += gui_state->input_state->mouse_offset;
  }
}


void Gui::request_cursor(GuiState *gui_state, GLFWcursor *cursor) {
  gui_state->requested_cursor = cursor;
}


void Gui::set_cursor(GuiState *gui_state) {
  Input::set_cursor(gui_state->input_state, gui_state->requested_cursor);
  gui_state->requested_cursor = nullptr;
}


void Gui::start_drawing(GuiState *gui_state) {
  glBindVertexArray(gui_state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, gui_state->vbo);
}


void Gui::push_vertices(GuiState *gui_state, real32 *vertices, uint32 n_vertices) {
  // VAO/VBO must have been bound by start_drawing()
  glBufferSubData(
    GL_ARRAY_BUFFER,
    GUI_VERTEX_SIZE * gui_state->n_vertices_pushed,
    GUI_VERTEX_SIZE * n_vertices,
    vertices
  );

  gui_state->n_vertices_pushed += n_vertices;
}


void Gui::render(GuiState *gui_state) {
  glUseProgram(gui_state->shader_asset->program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gui_state->texture_atlas.texture_name);

  if (!gui_state->shader_asset->did_set_texture_uniforms) {
    Shaders::set_int(gui_state->shader_asset, "atlas_texture", 0);
    gui_state->shader_asset->did_set_texture_uniforms = true;
  }

  glDrawArrays(GL_TRIANGLES, 0, gui_state->n_vertices_pushed);
  gui_state->n_vertices_pushed = 0;

  set_cursor(gui_state);
}


glm::vec2 Gui::get_text_dimensions(
  FontAsset *font_asset, const char *str
) {
  // NOTE: This returns the dimensions around the main body of the text.
  // This does not include descenders.
  real32 line_height = Fonts::font_unit_to_px(font_asset->height);
  real32 line_spacing = line_height * GUI_LINE_SPACING_FACTOR;
  real32 ascender = Fonts::font_unit_to_px(font_asset->ascender);

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
        max_x = glm::max(max_x, curr_x);
        curr_x = 0.0f;
        curr_y += line_spacing;
      }
      continue;
    }

    Character *character = font_asset->characters[c];

    if (!character) {
      log_warning("Could not get character: %c", c);
      continue;
    }

    curr_x += Fonts::frac_px_to_px(character->advance.x);
    curr_y += Fonts::frac_px_to_px(character->advance.y);
  }

  max_x = glm::max(max_x, curr_x);
  curr_y += line_height;

  return glm::vec2(max_x, curr_y);
}


glm::vec2 Gui::center_bb(
  glm::vec2 container_position,
  glm::vec2 container_dimensions,
  glm::vec2 element_dimensions
) {
  return glm::ceil(
    container_position + (container_dimensions / 2.0f) - (element_dimensions / 2.0f)
  );
}


glm::vec2 Gui::add_element_to_container(
  GuiContainer *container, glm::vec2 element_dimensions
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

  glm::vec2 new_element_position = container->next_element_position;
  glm::vec2 orthogonal_direction = glm::vec2(
    container->direction.y, container->direction.x
  );

  glm::vec2 required_space = element_dimensions;
  if (container->n_elements > 0) {
    required_space += (container->element_margin * container->direction);
  }

  container->content_dimensions = (
    (container->content_dimensions + required_space) * container->direction
  ) + (
    glm::max(container->content_dimensions, required_space) * orthogonal_direction
  );
  container->dimensions = container->content_dimensions +
    (container->padding * 2.0f) +
    glm::vec2(0.0f, container->title_bar_height);

  container->next_element_position = container->position +
    (
      (container->dimensions - container->padding + container->element_margin) *
      container->direction
    ) +
    (
      (container->padding + glm::vec2(0.0f, container->title_bar_height)) *
      orthogonal_direction
    );

  container->n_elements++;

  return new_element_position;
}


void Gui::draw_container(
  GuiState *gui_state,
  GuiContainer *container
) {
  draw_rect(
    gui_state,
    container->position,
    glm::vec2(
      container->dimensions.x, container->title_bar_height
    ),
    GUI_MAIN_DARKEN_COLOR
  );

  glm::vec2 text_dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body"),
    container->title
  );
  glm::vec2 centered_text_position = center_bb(
    container->position,
    glm::vec2(container->dimensions.x, container->title_bar_height),
    text_dimensions
  );
  glm::vec2 text_position = glm::vec2(
    container->position.x + container->padding.x,
    centered_text_position.y
  );
  draw_text_shadow(
    gui_state,
    "body",
    container->title,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );
  draw_text(
    gui_state,
    "body",
    container->title,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  draw_rect(
    gui_state,
    container->position + glm::vec2(0.0, container->title_bar_height),
    container->dimensions - glm::vec2(0.0, container->title_bar_height),
    GUI_WINDOW_BG_COLOR
  );
}


GuiContainer* Gui::make_container(
  GuiState *gui_state, const char *title, glm::vec2 position
) {
  GuiContainer *container = nullptr;
  for (uint32 idx = 0; idx < gui_state->containers.size; idx++) {
    GuiContainer *container_candidate = gui_state->containers[idx];
    if (strcmp(container_candidate->title, title) == 0) {
      container = container_candidate;
      break;
    }
  }

  if (container) {
    // Check if we need to set this container as being moved.
    if (
      Input::is_mouse_in_bb(
        gui_state->input_state,
        container->position,
        container->position + glm::vec2(
          container->dimensions.x, container->title_bar_height
        )
      ) &&
      Input::is_mouse_button_now_down(
        gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT
      )
    ) {
      gui_state->container_being_moved = container;
    }

    // Draw the container with the information from the previous frame
    // if there is anything in it.
    if (container->content_dimensions != glm::vec2(0.0f, 0.0f)) {
      draw_container(gui_state, container);
    }
  } else {
    container = gui_state->containers.push();
    container->title = title;
    container->position = position;
    container->direction = glm::vec2(0.0f, 1.0f);
    container->padding = glm::vec2(20.0f);
    container->title_bar_height = 40.0f;
    container->element_margin = 20.0f;
  }

  // In all cases, clear this container.
  container->n_elements = 0;
  container->dimensions = container->padding * 2.0f;
  container->content_dimensions = glm::vec2(0.0f, 0.0f);
  container->next_element_position = container->position +
    container->padding +
    glm::vec2(0.0f, container->title_bar_height);

  return container;
}


void Gui::draw_text(
  GuiState *gui_state,
  const char* font_name, const char *str,
  glm::vec2 position,
  glm::vec4 color
) {
  FontAsset *font_asset = Fonts::get_by_name(&gui_state->font_assets, font_name);

  real32 line_height = Fonts::font_unit_to_px(font_asset->height);
  real32 line_spacing = line_height * GUI_LINE_SPACING_FACTOR;
  real32 ascender = Fonts::font_unit_to_px(font_asset->ascender);

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
      log_warning("Could not get character: %c", c);
      continue;
    }

    real32 char_x = curr_x + character->bearing.x;
    real32 char_y = curr_y + (
      Fonts::font_unit_to_px(font_asset->height) - character->bearing.y
    );

    real32 tex_x = (real32)character->tex_coords.x / gui_state->texture_atlas.size.x;
    real32 tex_y = (real32)character->tex_coords.y / gui_state->texture_atlas.size.y;
    real32 tex_w = (real32)character->size.x / gui_state->texture_atlas.size.x;
    real32 tex_h = (real32)character->size.y / gui_state->texture_atlas.size.y;

    real32 w = (real32)character->size.x;
    real32 h = (real32)character->size.y;

    curr_x += Fonts::frac_px_to_px(character->advance.x);
    curr_y += Fonts::frac_px_to_px(character->advance.y);

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

    real32 vertices[GUI_VERTEX_LENGTH * 6] = {
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


void Gui::draw_text_shadow(
  GuiState *gui_state,
  const char* font_name, const char *str,
  glm::vec2 position,
  glm::vec4 color
) {
  draw_text(
    gui_state,
    font_name, str, position + GUI_TEXT_SHADOW_OFFSET,
    glm::vec4(0.0f, 0.0f, 0.0f, color.a * 0.2f)
  );
}


void Gui::draw_heading(
  GuiState *gui_state,
  const char *str,
  glm::vec4 color
) {
  glm::vec2 position = glm::vec2(
    center_bb(
      glm::vec2(0.0f, 0.0f),
      gui_state->window_dimensions,
      get_text_dimensions(
        Fonts::get_by_name(&gui_state->font_assets, "heading"),
        str
      )
    ).x,
    90.0f
  );
  draw_text_shadow(gui_state, "heading", str, position, color);
  draw_text(gui_state, "heading", str, position, color);
}


void Gui::draw_rect(
  GuiState *gui_state,
  glm::vec2 position,
  glm::vec2 dimensions,
  glm::vec4 color
) {
  // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
  // Flip the y axis before drawing.
  real32 x0 = position.x;
  real32 x1 = x0 + dimensions.x;
  real32 y0 = (real32)gui_state->window_dimensions.y - position.y;
  real32 y1 = y0 - dimensions.y;

  real32 vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a
  };
  push_vertices(gui_state, vertices, 6);
}


void Gui::draw_line(
  GuiState *gui_state,
  glm::vec2 start, glm::vec2 end,
  real32 thickness, glm::vec4 color
) {
  // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
  // Flip the y axis before drawing.
  glm::vec2 delta = glm::normalize(end - start) * thickness;

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

  real32 vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x3, y3, -1.0f, -1.0f, color.r, color.g, color.b, color.a
  };
  push_vertices(gui_state, vertices, 6);
}


void Gui::draw_frame(
  GuiState *gui_state,
  glm::vec2 position, glm::vec2 bottomright,
  glm::vec2 thickness, glm::vec4 color
) {
  draw_line(
    gui_state,
    glm::vec2(position.x, position.y),
    glm::vec2(bottomright.x, position.y),
    thickness.y,
    color
  );
  draw_line(
    gui_state,
    glm::vec2(position.x, bottomright.y - thickness.y),
    glm::vec2(bottomright.x, bottomright.y - thickness.y),
    thickness.y,
    color
  );
  draw_line(
    gui_state,
    glm::vec2(position.x, position.y),
    glm::vec2(position.x, bottomright.y),
    thickness.x,
    color
  );
  draw_line(
    gui_state,
    glm::vec2(bottomright.x - thickness.x, position.y),
    glm::vec2(bottomright.x - thickness.x, bottomright.y),
    thickness.x,
    color
  );
}


bool32 Gui::draw_toggle(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text,
  bool32 *toggle_state
) {
  bool32 is_pressed = false;

  glm::vec2 text_dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  glm::vec2 button_dimensions = GUI_TOGGLE_BUTTON_SIZE +
    GUI_BUTTON_DEFAULT_BORDER * 2.0f;
  glm::vec2 dimensions = glm::vec2(
    button_dimensions.x + GUI_TOGGLE_SPACING + text_dimensions.x,
    glm::max(button_dimensions.y, text_dimensions.y)
  );

  glm::vec2 position = add_element_to_container(container, dimensions);

  glm::vec2 button_bottomright = position + button_dimensions;
  glm::vec2 text_centered_position = center_bb(
    position, button_dimensions, text_dimensions
  );
  glm::vec2 text_position = glm::vec2(
    position.x + button_dimensions.x + GUI_TOGGLE_SPACING,
    text_centered_position.y
  );

  glm::vec4 button_color;
  if (*toggle_state) {
    button_color = GUI_MAIN_COLOR;
  } else {
    button_color = GUI_LIGHT_COLOR;
  }

  if (Input::is_mouse_in_bb(gui_state->input_state, position, button_bottomright)) {
    request_cursor(gui_state, gui_state->input_state->hand_cursor);
    if (*toggle_state) {
      button_color = GUI_MAIN_HOVER_COLOR;
    } else {
      button_color = GUI_LIGHT_HOVER_COLOR;
    }

    if (Input::is_mouse_button_now_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (Input::is_mouse_button_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      if (*toggle_state) {
        button_color = GUI_MAIN_ACTIVE_COLOR;
      } else {
        button_color = GUI_LIGHT_ACTIVE_COLOR;
      }
    }
  }

  draw_frame(
    gui_state,
    position,
    button_bottomright,
    GUI_TOGGLE_BUTTON_DEFAULT_BORDER,
    GUI_LIGHT_DARKEN_COLOR
  );
  draw_rect(
    gui_state,
    position + GUI_TOGGLE_BUTTON_DEFAULT_BORDER,
    button_dimensions - (GUI_TOGGLE_BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    gui_state,
    "body", text,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


void Gui::draw_named_value(
  GuiState *gui_state,
  GuiContainer *container,
  const char *name_text,
  const char *value_text
) {
  glm::vec2 name_text_dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body-bold"),
    name_text
  );
  glm::vec2 value_text_dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body"),
    value_text
  );
  // Sometimes we draw a value which is a rapidly changing number.
  // We don't want to container to wobble in size back and forth, so we round
  // the size of the value text to the next multiple of 50.
  value_text_dimensions.x = Util::round_to_nearest_multiple(
    value_text_dimensions.x, 50.0f
  );
  glm::vec2 dimensions = glm::vec2(
    value_text_dimensions.x + GUI_NAMED_VALUE_NAME_WIDTH,
    glm::max(name_text_dimensions.y, value_text_dimensions.y)
  );

  glm::vec2 position = add_element_to_container(container, dimensions);

  draw_text(
    gui_state,
    "body-bold", name_text,
    position,
    GUI_LIGHT_TEXT_COLOR
  );

  glm::vec2 value_text_position = position +
    glm::vec2(GUI_NAMED_VALUE_NAME_WIDTH, 0.0f);
  draw_text(
    gui_state,
    "body", value_text,
    value_text_position,
    GUI_LIGHT_TEXT_COLOR
  );
}


void Gui::draw_body_text(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text
) {
  glm::vec2 dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  glm::vec2 position = add_element_to_container(container, dimensions);
  draw_text(gui_state, "body", text, position, GUI_LIGHT_TEXT_COLOR);
}


bool32 Gui::draw_button(
  GuiState *gui_state,
  GuiContainer *container,
  const char *text
) {
  bool32 is_pressed = false;

  glm::vec2 text_dimensions = get_text_dimensions(
    Fonts::get_by_name(&gui_state->font_assets, "body"),
    text
  );
  glm::vec2 button_dimensions = text_dimensions +
    GUI_BUTTON_AUTOSIZE_PADDING +
    GUI_BUTTON_DEFAULT_BORDER * 2.0f;

  glm::vec2 position = add_element_to_container(container, button_dimensions);

  glm::vec2 bottomright = position + button_dimensions;
  glm::vec2 text_position = center_bb(position, button_dimensions, text_dimensions);

  glm::vec4 button_color = GUI_MAIN_COLOR;

  if (Input::is_mouse_in_bb(gui_state->input_state, position, bottomright)) {
    request_cursor(gui_state, gui_state->input_state->hand_cursor);
    button_color = GUI_MAIN_HOVER_COLOR;

    if (Input::is_mouse_button_now_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (Input::is_mouse_button_down(gui_state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
      button_color = GUI_MAIN_ACTIVE_COLOR;
    }
  }

  draw_frame(
    gui_state,
    position,
    bottomright,
    GUI_BUTTON_DEFAULT_BORDER,
    GUI_MAIN_DARKEN_COLOR
  );
  draw_rect(
    gui_state,
    position + GUI_BUTTON_DEFAULT_BORDER,
    button_dimensions - (GUI_BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    gui_state,
    "body", text,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


GuiState* Gui::init_gui_state(
  GuiState* gui_state,
  MemoryPool *memory_pool,
  Array<ShaderAsset> *shader_assets,
  InputState *input_state,
  uint32 window_width, uint32 window_height
) {
  gui_state->font_assets = Array<FontAsset>(
    memory_pool, 8, "font_assets"
  );
  gui_state->containers = Array<GuiContainer>(
    memory_pool, 32, "gui_containers"
  );
  gui_state->input_state = input_state;
  gui_state->window_dimensions = glm::vec2(window_width, window_height);
  Materials::init_texture_atlas(&gui_state->texture_atlas, glm::ivec2(2000, 2000));

  // Shaders
  {
    gui_state->shader_asset = Shaders::init_shader_asset(
      (ShaderAsset*)(shader_assets->push()),
      "gui_generic", ShaderType::standard,
      "gui_generic.vert", "gui_generic.frag", ""
    );
  }

  // Fonts
  {
    FT_Library ft_library;

    if (FT_Init_FreeType(&ft_library)) {
      log_error("Could not init FreeType");
      return nullptr;
    }

    Fonts::init_font_asset(
      (FontAsset*)(gui_state->font_assets.push()),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "body", GUI_MAIN_FONT_REGULAR, 18
    );

    Fonts::init_font_asset(
      (FontAsset*)(gui_state->font_assets.push()),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "body-bold", GUI_MAIN_FONT_BOLD, 18
    );

    Fonts::init_font_asset(
      (FontAsset*)(gui_state->font_assets.push()),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "heading", GUI_MAIN_FONT_REGULAR, 42
    );

    Fonts::init_font_asset(
      (FontAsset*)(gui_state->font_assets.push()),
      memory_pool, &gui_state->texture_atlas,
      &ft_library, "title", GUI_MAIN_FONT_REGULAR, 64
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
      GL_ARRAY_BUFFER, GUI_VERTEX_SIZE * 6 * GUI_N_MAX_VERTICES,
      NULL, GL_DYNAMIC_DRAW
    );

    uint32 location;

    // position (vec2)
    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 2, GL_FLOAT, GL_FALSE, GUI_VERTEX_SIZE, (void*)(0)
    );

    // tex_coords (vec2)
    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 2, GL_FLOAT, GL_FALSE, GUI_VERTEX_SIZE, (void*)(2 * sizeof(real32))
    );

    // color (vec4)
    location = 2;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 4, GL_FLOAT, GL_FALSE, GUI_VERTEX_SIZE, (void*)(4 * sizeof(real32))
    );
  }

  return gui_state;
}
