void GuiManager::update_screen_dimensions(
  uint32 new_window_width, uint32 new_window_height
) {
  this->window_dimensions = glm::vec2(new_window_width, new_window_height);
}


void GuiManager::request_cursor(GLFWcursor *cursor) {
  this->requested_cursor = cursor;
}


void GuiManager::set_cursor() {
  this->input_manager->set_cursor(this->requested_cursor);
  this->requested_cursor = nullptr;
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
  real32 curr_x = start_x;
  real32 curr_y = start_y;
  size_t str_length = strlen(str);

  for (uint32 idx = 0; idx < str_length; idx++) {
    char c = str[idx];

    if (c < 32) {
      if (c == '\n') {
        curr_x = curr_x;
        curr_y += line_spacing;
      }
      continue;
    }

    Character *character = font_asset->characters.get(c);

    if (!character) {
      log_warning("Could not get character: %c", c);
      continue;
    }

    curr_x += font_asset->frac_px_to_px(character->advance.x);
    curr_y += font_asset->frac_px_to_px(character->advance.y);
  }

  curr_y += line_height;

  return glm::vec2(curr_x, curr_y);
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

  container->dimensions = (
    (container->dimensions + required_space) * container->direction
  ) + (
    glm::max(
      container->dimensions,
      required_space + (container->padding * 2.0f)
    ) * orthogonal_direction
  );

  container->next_element_position = container->position +
    (
      (container->dimensions - container->padding + container->element_margin) *
      container->direction
    ) +
    (container->padding * orthogonal_direction);

  container->n_elements++;

  return new_element_position;
}


GuiContainer GuiManager::make_container(
  const char *title, glm::vec2 position
) {
  GuiContainer container;
  container.title = title;
  container.position = position;
  container.direction = glm::vec2(0.0f, 1.0f);
  container.padding = glm::vec2(20.0f);
  container.n_elements = 0;
  container.element_margin = 20.0f;
  container.dimensions = container.padding * 2.0f;
  container.next_element_position = position + container.padding;
  return container;
}


void GuiManager::draw_container(GuiContainer *container) {
  draw_rect(
    container->position,
    container->dimensions,
    glm::vec4(1.0f, 0.0f, 0.0f, 0.2f)
  );
  draw_rect(
    container->position + container->padding,
    container->dimensions - (container->padding * 2.0f),
    glm::vec4(1.0f, 0.0f, 1.0f, 0.2f)
  );
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

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->text_shader_asset->program);
  if (!this->text_shader_asset->did_set_texture_uniforms) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_asset->texture);
    this->text_shader_asset->set_int("font_atlas_texture", 0);
    this->text_shader_asset->did_set_texture_uniforms;
  }

  // NOTE: When changing this code, remember that the text positioning logic
  // needs to be replicated in `get_text_dimensions()`!
  real32 start_x = position.x;
  real32 start_y = position.y - (line_height - ascender);
  real32 curr_x = start_x;
  real32 curr_y = start_y;
  size_t str_length = strlen(str);
  size_t str_printable_length = 0;

  for (uint32 idx = 0; idx < str_length; idx++) {
    char c = str[idx];

    if (c < 32) {
      if (c == '\n') {
        curr_x = start_x;
        curr_y += line_spacing;
      }
      continue;
    }

    Character *character = font_asset->characters.get(c);

    if (!character) {
      log_warning("Could not get character: %c", c);
      continue;
    }

    real32 char_x = curr_x + character->bearing.x;
    real32 char_y = curr_y + (
      font_asset->font_unit_to_px(font_asset->height) - character->bearing.y
    );

    real32 tex_x = character->texture_x;
    real32 tex_w = (real32)character->size.x / font_asset->atlas_width;
    real32 tex_h = (real32)character->size.y / font_asset->atlas_height;

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

    real32 tex_x0 = tex_x;
    real32 tex_x1 = tex_x0 + tex_w;
    real32 tex_y0 = 0;
    real32 tex_y1 = tex_y0 + tex_h;

    real32 character_vertices[GUI_VERTEX_LENGTH * 6] = {
      x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
      x0, y1, tex_x0, tex_y1, color.r, color.g, color.b, color.a,
      x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
      x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
      x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
      x1, y0, tex_x1, tex_y0, color.r, color.g, color.b, color.a
    };
    glBufferSubData(
      GL_ARRAY_BUFFER,
      sizeof(character_vertices) * str_printable_length,
      sizeof(character_vertices),
      character_vertices
    );
    str_printable_length += 1;
  }

  glDrawArrays(GL_TRIANGLES, 0, 6 * (uint32)str_printable_length);
}


void GuiManager::draw_heading(
  const char* font_name, const char *str,
  glm::vec4 color
) {
  glm::vec2 position = glm::vec2(
    center_bb(
      glm::vec2(0.0f, 0.0f),
      window_dimensions,
      get_text_dimensions(font_name, str)
    ).x,
    90.0f
  );
  draw_text(font_name, str, position, color);
}


void GuiManager::draw_rect(
  glm::vec2 position, glm::vec2 dimensions, glm::vec4 color
) {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->generic_shader_asset->program);

  // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
  // Flip the y axis before drawing.
  real32 x0 = position.x;
  real32 x1 = x0 + dimensions.x;
  real32 y0 = (real32)this->window_dimensions.y - position.y;
  real32 y1 = y0 - dimensions.y;

  real32 character_vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, 0, 0, color.r, color.g, color.b, color.a,
    x0, y1, 0, 0, color.r, color.g, color.b, color.a,
    x1, y1, 0, 0, color.r, color.g, color.b, color.a,
    x0, y0, 0, 0, color.r, color.g, color.b, color.a,
    x1, y1, 0, 0, color.r, color.g, color.b, color.a,
    x1, y0, 0, 0, color.r, color.g, color.b, color.a
  };
  glBufferSubData(
    GL_ARRAY_BUFFER,
    0,
    sizeof(character_vertices),
    character_vertices
  );

  glDrawArrays(GL_TRIANGLES, 0, 6);
}


void GuiManager::draw_line(
  glm::vec2 start, glm::vec2 end,
  real32 thickness, glm::vec4 color
) {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->generic_shader_asset->program);

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

  real32 character_vertices[GUI_VERTEX_LENGTH * 6] = {
    x0, y0, 0, 0, color.r, color.g, color.b, color.a,
    x1, y1, 0, 0, color.r, color.g, color.b, color.a,
    x2, y2, 0, 0, color.r, color.g, color.b, color.a,
    x0, y0, 0, 0, color.r, color.g, color.b, color.a,
    x2, y2, 0, 0, color.r, color.g, color.b, color.a,
    x3, y3, 0, 0, color.r, color.g, color.b, color.a
  };
  glBufferSubData(
    GL_ARRAY_BUFFER,
    0,
    sizeof(character_vertices),
    character_vertices
  );

  glDrawArrays(GL_TRIANGLES, 0, 6);
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


bool32 GuiManager::draw_button(
  GuiContainer *container,
  const char *text
) {
  const char *font = "body";
  bool32 is_pressed = false;

  glm::vec2 text_dimensions = get_text_dimensions(font, text);
  glm::vec2 button_dimensions = text_dimensions +
    GUI_BUTTON_AUTOSIZE_PADDING +
    GUI_BUTTON_DEFAULT_BORDER * 2.0f;

  glm::vec2 position = add_element_to_container(container, button_dimensions);

  glm::vec2 bottomright = position + button_dimensions;
  glm::vec2 text_position = center_bb(position, button_dimensions, text_dimensions);

  glm::vec4 color = GUI_BUTTON_COLOR;

  if (this->input_manager->is_mouse_in_bb(position, bottomright)) {
    this->request_cursor(this->input_manager->hand_cursor);
    color = GUI_BUTTON_HOVER_COLOR;

    if (this->input_manager->is_mouse_button_now_down(GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (this->input_manager->is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
      color = GUI_BUTTON_ACTIVE_COLOR;
    }
  }

  draw_frame(
    position,
    bottomright,
    GUI_BUTTON_DEFAULT_BORDER,
    GUI_BUTTON_BORDER_COLOR
  );
  draw_rect(
    position + GUI_BUTTON_DEFAULT_BORDER,
    button_dimensions - (GUI_BUTTON_DEFAULT_BORDER * 2.0f),
    color
  );
  draw_text(
    font, text,
    text_position,
    GUI_BUTTON_TEXT_COLOR
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
  memory(memory),
  input_manager(input_manager),
  window_dimensions(window_width, window_height)
{
  // Shaders
  {
    this->text_shader_asset = new(shader_assets->push()) ShaderAsset(
      this->memory, "gui_text", SHADER_STANDARD,
      SHADER_DIR"gui_text.vert", SHADER_DIR"gui_text.frag", nullptr
    );

    this->generic_shader_asset = new(shader_assets->push()) ShaderAsset(
      this->memory, "gui_generic", SHADER_STANDARD,
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
      this->memory, &ft_library, "body", GUI_DEFAULT_FONT, 18
    );

    new(this->font_assets.push()) FontAsset(
      this->memory, &ft_library, "heading", GUI_DEFAULT_FONT, 42
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
      GL_ARRAY_BUFFER, GUI_VERTEX_SIZE * 6 * GUI_N_MAX_CHARACTERS_PER_DRAW,
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
