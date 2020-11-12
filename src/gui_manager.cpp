void GuiManager::update_screen_dimensions(
  uint32 new_window_width, uint32 new_window_height
) {
  this->window_width = new_window_width;
  this->window_height = new_window_height;
}


void GuiManager::request_cursor(GLFWcursor *cursor) {
  this->requested_cursor = cursor;
}


void GuiManager::set_cursor() {
  this->input_manager->set_cursor(this->requested_cursor);
  this->requested_cursor = nullptr;
}


void GuiManager::draw_text(
  const char* font_name, const char *str,
  glm::vec2 topleft,
  real32 scale, glm::vec4 color
) {
  FontAsset *font_asset = FontAsset::get_by_name(&this->font_assets, font_name);

  uint16 line_height = (uint16)(
    (real32)font_asset->font_unit_to_px(font_asset->height) * LINE_HEIGHT_FACTOR
  );

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->text_shader_asset->program);
  if (!this->text_shader_asset->did_set_texture_uniforms) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_asset->texture);
    this->text_shader_asset->set_int("font_atlas_texture", 0);
    this->text_shader_asset->did_set_texture_uniforms;
  }

  real32 curr_x = topleft.x;
  real32 curr_y = topleft.y;
  size_t str_length = strlen(str);
  size_t str_printable_length = 0;

  for (uint32 idx = 0; idx < str_length; idx++) {
    char c = str[idx];

    if (c < 32) {
      if (c == '\n') {
        curr_x = topleft.x;
        curr_y += line_height;
      }
      continue;
    }

    Character *character = font_asset->characters.get(c);

    if (!character) {
      log_warning("Could not get character: %c", c);
      continue;
    }

    real32 char_x = curr_x + character->bearing.x * scale;
    real32 char_y = curr_y + (
      font_asset->font_unit_to_px(font_asset->height) - character->bearing.y
    ) * scale;

    real32 tex_x = character->texture_x;
    real32 tex_w = (real32)character->size.x / font_asset->atlas_width;
    real32 tex_h = (real32)character->size.y / font_asset->atlas_height;

    real32 w = character->size.x * scale;
    real32 h = character->size.y * scale;

    curr_x += font_asset->frac_px_to_px(character->advance.x) * scale;
    curr_y += font_asset->frac_px_to_px(character->advance.y) * scale;

    // Skip glyphs with no pixels, like spaces.
    if (w <= 0 || h <= 0) {
      continue;
    }

    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    real32 x0 = char_x;
    real32 x1 = x0 + w;
    real32 y0 = (real32)this->window_height - char_y;
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


void GuiManager::draw_rect(
  glm::vec2 topleft, real32 w, real32 h, glm::vec4 color
) {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->generic_shader_asset->program);

  // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
  // Flip the y axis before drawing.
  real32 x0 = topleft.x;
  real32 x1 = x0 + w;
  real32 y0 = (real32)this->window_height - topleft.y;
  real32 y1 = y0 - h;

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
  real32 y0 = this->window_height - start.y;
  real32 x1 = start.x;
  real32 y1 = this->window_height - start.y - delta.x;
  real32 x2 = end.x;
  real32 y2 = this->window_height - end.y - delta.x;
  real32 x3 = end.x + delta.y;
  real32 y3 = this->window_height - end.y;

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
  glm::vec2 topleft, glm::vec2 bottomright,
  real32 thickness, glm::vec4 color
) {
  draw_line(
    glm::vec2(topleft.x - thickness, topleft.y - thickness),
    glm::vec2(bottomright.x + thickness, topleft.y - thickness),
    thickness,
    color
  );
  draw_line(
    glm::vec2(topleft.x - thickness, bottomright.y),
    glm::vec2(bottomright.x + thickness, bottomright.y),
    thickness,
    color
  );
  draw_line(
    glm::vec2(topleft.x - thickness, topleft.y - thickness),
    glm::vec2(topleft.x - thickness, bottomright.y + thickness),
    thickness,
    color
  );
  draw_line(
    glm::vec2(bottomright.x, topleft.y - thickness),
    glm::vec2(bottomright.x, bottomright.y + thickness),
    thickness,
    color
  );
}


bool32 GuiManager::draw_button(
  glm::vec2 topleft, real32 w, real32 h,
  const char *text,
  real32 border_thickness
) {
  bool32 is_pressed = false;
  glm::vec2 bottomright = topleft + glm::vec2(w, h);

  glm::vec4 color = BUTTON_COLOR;

  if (this->input_manager->is_mouse_in_bb(topleft, bottomright)) {
    this->request_cursor(this->input_manager->hand_cursor);
    color = BUTTON_HOVER_COLOR;

    if (this->input_manager->is_mouse_button_now_down(GLFW_MOUSE_BUTTON_LEFT)) {
      is_pressed = true;
    }

    if (this->input_manager->is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
      color = BUTTON_ACTIVE_COLOR;
    }
  }

  draw_frame(
    topleft,
    bottomright,
    border_thickness,
    BUTTON_BORDER_COLOR
  );
  draw_rect(topleft, w, h, color);
  draw_text(
    "main-font", text,
    topleft + glm::vec2(30.0f, 7.0f),
    1.0f,
    BUTTON_TEXT_COLOR
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
  window_width(window_width),
  window_height(window_height)
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
      this->memory, &ft_library, "main-font", DEFAULT_FONT, DEFAULT_FONT_SIZE
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
      GL_ARRAY_BUFFER, GUI_VERTEX_SIZE * 6 * N_MAX_CHARACTERS_PER_DRAW,
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
