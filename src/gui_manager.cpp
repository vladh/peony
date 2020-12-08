void GuiManager::update_screen_dimensions(
  uint32 new_window_width, uint32 new_window_height
) {
  this->window_dimensions = glm::vec2(new_window_width, new_window_height);
}


void GuiManager::update_mouse_button() {
  if (this->input_manager->is_mouse_button_now_up(GLFW_MOUSE_BUTTON_LEFT)) {
    this->container_being_moved = nullptr;
  }
}


void GuiManager::update_mouse() {
  if (this->container_being_moved) {
    this->container_being_moved->position += this->input_manager->mouse_offset;
  }
}


void GuiManager::request_cursor(GLFWcursor *cursor) {
  this->requested_cursor = cursor;
}


void GuiManager::set_cursor() {
  this->input_manager->set_cursor(this->requested_cursor);
  this->requested_cursor = nullptr;
}


void GuiManager::start_drawing() {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
}


void GuiManager::push_vertices(real32 *vertices, uint32 n_vertices) {
  // VAO/VBO must have been bound by start_drawing()
  glBufferSubData(
    GL_ARRAY_BUFFER,
    GUI_VERTEX_SIZE * this->n_vertices_pushed,
    GUI_VERTEX_SIZE * n_vertices,
    vertices
  );

  this->n_vertices_pushed += n_vertices;
}


void GuiManager::render() {
  glUseProgram(this->shader_asset->program);
  if (!this->shader_asset->did_set_texture_uniforms) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_atlas.texture_name);
    this->shader_asset->set_int("atlas_texture", 0);
    this->shader_asset->did_set_texture_uniforms;
  }

  glDrawArrays(GL_TRIANGLES, 0, this->n_vertices_pushed);
  this->n_vertices_pushed = 0;

  set_cursor();
}


glm::vec2 GuiManager::get_text_dimensions(
  const char* font_name, const char *str
) {
  // NOTE: This returns the dimensions around the main body of the text.
  // This does not include descenders.
  FontAsset *font_asset = FontAsset::get_by_name(&this->font_assets, font_name);

  real32 line_height = font_asset->font_unit_to_px(font_asset->height);
  real32 line_spacing = line_height * GUI_LINE_SPACING_FACTOR;
  real32 ascender = font_asset->font_unit_to_px(font_asset->ascender);

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

    curr_x += font_asset->frac_px_to_px(character->advance.x);
    curr_y += font_asset->frac_px_to_px(character->advance.y);
  }

  max_x = glm::max(max_x, curr_x);
  curr_y += line_height;

  return glm::vec2(max_x, curr_y);
}


glm::vec2 GuiManager::center_bb(
  glm::vec2 container_position,
  glm::vec2 container_dimensions,
  glm::vec2 element_dimensions
) {
  return glm::ceil(
    container_position + (container_dimensions / 2.0f) - (element_dimensions / 2.0f)
  );
}


glm::vec2 GuiManager::add_element_to_container(
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


void GuiManager::draw_container(GuiContainer *container) {
  draw_rect(
    container->position,
    glm::vec2(
      container->dimensions.x, container->title_bar_height
    ),
    GUI_MAIN_DARKEN_COLOR
  );

  glm::vec2 text_dimensions = get_text_dimensions(
    "body",
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
    "body",
    container->title,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );
  draw_text(
    "body",
    container->title,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  draw_rect(
    container->position + glm::vec2(0.0, container->title_bar_height),
    container->dimensions - glm::vec2(0.0, container->title_bar_height),
    GUI_WINDOW_BG_COLOR
  );
}


GuiContainer* GuiManager::make_container(
  const char *title, glm::vec2 position
) {
  GuiContainer *container = nullptr;
  for (uint32 idx = 0; idx < this->containers.size; idx++) {
    GuiContainer *container_candidate = this->containers[idx];
    if (strcmp(container_candidate->title, title) == 0) {
      container = container_candidate;
      break;
    }
  }

  if (container) {
    // Check if we need to set this container as being moved.
    if (
      this->input_manager->is_mouse_in_bb(
        container->position,
        container->position + glm::vec2(
          container->dimensions.x, container->title_bar_height
        )
      ) &&
      this->input_manager->is_mouse_button_now_down(GLFW_MOUSE_BUTTON_LEFT)
    ) {
      this->container_being_moved = container;
    }

    // Draw the container with the information from the previous frame
    // if there is anything in it.
    if (container->content_dimensions != glm::vec2(0.0f, 0.0f)) {
      draw_container(container);
    }
  } else {
    container = this->containers.push();
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


void GuiManager::draw_text(
  const char* font_name, const char *str,
  glm::vec2 position,
  glm::vec4 color
) {
  FontAsset *font_asset = FontAsset::get_by_name(&this->font_assets, font_name);

  real32 line_height = font_asset->font_unit_to_px(font_asset->height);
  real32 line_spacing = line_height * GUI_LINE_SPACING_FACTOR;
  real32 ascender = font_asset->font_unit_to_px(font_asset->ascender);

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
      font_asset->font_unit_to_px(font_asset->height) - character->bearing.y
    );

    real32 tex_x = (real32)character->tex_coords.x / this->texture_atlas.size.x;
    real32 tex_y = (real32)character->tex_coords.y / this->texture_atlas.size.y;
    real32 tex_w = (real32)character->size.x / this->texture_atlas.size.x;
    real32 tex_h = (real32)character->size.y / this->texture_atlas.size.y;

    real32 w = (real32)character->size.x;
    real32 h = (real32)character->size.y;

    curr_x += font_asset->frac_px_to_px(character->advance.x);
    curr_y += font_asset->frac_px_to_px(character->advance.y);

    // Skip glyphs with no pixels, like spaces.
    if (w <= 0 || h <= 0) {
      continue;
    }

    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    real32 x0 = char_x;
    real32 x1 = x0 + w;
    real32 y0 = (real32)this->window_dimensions.y - char_y;
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
    push_vertices(vertices, 6);
  }
}


void GuiManager::draw_text_shadow(
  const char* font_name, const char *str,
  glm::vec2 position,
  glm::vec4 color
) {
  draw_text(
    font_name, str, position + GUI_TEXT_SHADOW_OFFSET,
    glm::vec4(0.0f, 0.0f, 0.0f, color.a * 0.2f)
  );
}


void GuiManager::draw_heading(
  const char *str,
  glm::vec4 color
) {
  glm::vec2 position = glm::vec2(
    center_bb(
      glm::vec2(0.0f, 0.0f),
      window_dimensions,
      get_text_dimensions("heading", str)
    ).x,
    90.0f
  );
  draw_text_shadow("heading", str, position, color);
  draw_text("heading", str, position, color);
}


void GuiManager::draw_rect(
  glm::vec2 position, glm::vec2 dimensions, glm::vec4 color
) {
  // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
  // Flip the y axis before drawing.
  real32 x0 = position.x;
  real32 x1 = x0 + dimensions.x;
  real32 y0 = (real32)this->window_dimensions.y - position.y;
  real32 y1 = y0 - dimensions.y;

  real32 vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a
  };
  push_vertices(vertices, 6);
}


void GuiManager::draw_line(
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
  real32 y0 = this->window_dimensions.y - start.y;
  real32 x1 = start.x;
  real32 y1 = this->window_dimensions.y - start.y - delta.x;
  real32 x2 = end.x;
  real32 y2 = this->window_dimensions.y - end.y - delta.x;
  real32 x3 = end.x + delta.y;
  real32 y3 = this->window_dimensions.y - end.y;

  real32 vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
    x3, y3, -1.0f, -1.0f, color.r, color.g, color.b, color.a
  };
  push_vertices(vertices, 6);
}


void GuiManager::draw_frame(
  glm::vec2 position, glm::vec2 bottomright,
  glm::vec2 thickness, glm::vec4 color
) {
  draw_line(
    glm::vec2(position.x, position.y),
    glm::vec2(bottomright.x, position.y),
    thickness.y,
    color
  );
  draw_line(
    glm::vec2(position.x, bottomright.y - thickness.y),
    glm::vec2(bottomright.x, bottomright.y - thickness.y),
    thickness.y,
    color
  );
  draw_line(
    glm::vec2(position.x, position.y),
    glm::vec2(position.x, bottomright.y),
    thickness.x,
    color
  );
  draw_line(
    glm::vec2(bottomright.x - thickness.x, position.y),
    glm::vec2(bottomright.x - thickness.x, bottomright.y),
    thickness.x,
    color
  );
}


bool32 GuiManager::draw_toggle(
  GuiContainer *container,
  const char *text,
  bool32 *toggle_state
) {
  bool32 is_pressed = false;

  glm::vec2 text_dimensions = get_text_dimensions("body", text);
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

  if (this->input_manager->is_mouse_in_bb(position, button_bottomright)) {
    this->request_cursor(this->input_manager->hand_cursor);
    if (*toggle_state) {
      button_color = GUI_MAIN_HOVER_COLOR;
    } else {
      button_color = GUI_LIGHT_HOVER_COLOR;
    }

    if (this->input_manager->is_mouse_button_now_down(GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (this->input_manager->is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
      if (*toggle_state) {
        button_color = GUI_MAIN_ACTIVE_COLOR;
      } else {
        button_color = GUI_LIGHT_ACTIVE_COLOR;
      }
    }
  }

  draw_frame(
    position,
    button_bottomright,
    GUI_TOGGLE_BUTTON_DEFAULT_BORDER,
    GUI_LIGHT_DARKEN_COLOR
  );
  draw_rect(
    position + GUI_TOGGLE_BUTTON_DEFAULT_BORDER,
    button_dimensions - (GUI_TOGGLE_BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    "body", text,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


void GuiManager::draw_named_value(
  GuiContainer *container,
  const char *name_text,
  const char *value_text
) {
  glm::vec2 name_text_dimensions = get_text_dimensions("body-bold", name_text);
  glm::vec2 value_text_dimensions = get_text_dimensions("body", value_text);
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
    "body-bold", name_text,
    position,
    GUI_LIGHT_TEXT_COLOR
  );

  glm::vec2 value_text_position = position +
    glm::vec2(GUI_NAMED_VALUE_NAME_WIDTH, 0.0f);
  draw_text(
    "body", value_text,
    value_text_position,
    GUI_LIGHT_TEXT_COLOR
  );
}


void GuiManager::draw_body_text(
  GuiContainer *container,
  const char *text
) {
  glm::vec2 dimensions = get_text_dimensions("body", text);
  glm::vec2 position = add_element_to_container(container, dimensions);
  draw_text("body", text, position, GUI_LIGHT_TEXT_COLOR);
}


bool32 GuiManager::draw_button(
  GuiContainer *container,
  const char *text
) {
  bool32 is_pressed = false;

  glm::vec2 text_dimensions = get_text_dimensions("body", text);
  glm::vec2 button_dimensions = text_dimensions +
    GUI_BUTTON_AUTOSIZE_PADDING +
    GUI_BUTTON_DEFAULT_BORDER * 2.0f;

  glm::vec2 position = add_element_to_container(container, button_dimensions);

  glm::vec2 bottomright = position + button_dimensions;
  glm::vec2 text_position = center_bb(position, button_dimensions, text_dimensions);

  glm::vec4 button_color = GUI_MAIN_COLOR;

  if (this->input_manager->is_mouse_in_bb(position, bottomright)) {
    this->request_cursor(this->input_manager->hand_cursor);
    button_color = GUI_MAIN_HOVER_COLOR;

    if (this->input_manager->is_mouse_button_now_down(GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (this->input_manager->is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
      button_color = GUI_MAIN_ACTIVE_COLOR;
    }
  }

  draw_frame(
    position,
    bottomright,
    GUI_BUTTON_DEFAULT_BORDER,
    GUI_MAIN_DARKEN_COLOR
  );
  draw_rect(
    position + GUI_BUTTON_DEFAULT_BORDER,
    button_dimensions - (GUI_BUTTON_DEFAULT_BORDER * 2.0f),
    button_color
  );
  draw_text(
    "body", text,
    text_position,
    GUI_LIGHT_TEXT_COLOR
  );

  return is_pressed;
}


GuiManager::GuiManager(
  Memory *memory, Array<ShaderAsset> *shader_assets,
  InputManager *input_manager,
  uint32 window_width, uint32 window_height
) :
  font_assets(
    Array<FontAsset>(
      &memory->asset_memory_pool, 8, "font_assets"
    )
  ),
  containers(
    Array<GuiContainer>(
      &memory->asset_memory_pool, 32, "gui_containers"
    )
  ),
  container_being_moved(nullptr),
  memory(memory),
  input_manager(input_manager),
  window_dimensions(window_width, window_height),
  n_vertices_pushed(0),
  texture_atlas(glm::ivec2(2000, 2000))
{
  // Shaders
  {
    this->shader_asset = new(shader_assets->push()) ShaderAsset(
      this->memory, "gui_generic", ShaderType::standard,
      SHADER_DIR"gui_generic.vert", SHADER_DIR"gui_generic.frag", nullptr
    );
  }

  // Fonts
  {
    FT_Library ft_library;

    if (FT_Init_FreeType(&ft_library)) {
      log_error("Could not init FreeType");
      return;
    }

    new(this->font_assets.push()) FontAsset(
      this->memory, &this->texture_atlas,
      &ft_library, "body", GUI_MAIN_FONT_REGULAR, 18
    );

    new(this->font_assets.push()) FontAsset(
      this->memory, &this->texture_atlas,
      &ft_library, "body-bold", GUI_MAIN_FONT_BOLD, 18
    );

    new(this->font_assets.push()) FontAsset(
      this->memory, &this->texture_atlas,
      &ft_library, "heading", GUI_MAIN_FONT_REGULAR, 42
    );

    new(this->font_assets.push()) FontAsset(
      this->memory, &this->texture_atlas,
      &ft_library, "title", GUI_MAIN_FONT_REGULAR, 64
    );

    FT_Done_FreeType(ft_library);
  }

  // VAO
  {
    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
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
}
