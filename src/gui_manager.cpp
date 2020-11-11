constexpr real32 LINE_HEIGHT_FACTOR = 1.66f;
constexpr uint32 N_MAX_CHARACTERS_PER_DRAW = 1024;
constexpr const char *DEFAULT_FONT = "resources/fonts/iosevka-regular.ttf";
constexpr uint32 DEFAULT_FONT_SIZE = 18;
constexpr uint32 GUI_VERTEX_LENGTH = 8;
constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * GUI_VERTEX_LENGTH;


void GuiManager::draw_text(
  const char* font_name, const char *str,
  real32 start_x, real32 start_y,
  real32 scale, glm::vec4 color
) {
  FontAsset *font_asset = FontAsset::get_by_name(&this->font_assets, font_name);

  uint16 line_height = (uint16)((real32)font_asset->font_size * LINE_HEIGHT_FACTOR);

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->shader_asset->program);
  if (!this->shader_asset->did_set_texture_uniforms) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_asset->texture);
    this->shader_asset->set_int("font_atlas_texture", 0);
    this->shader_asset->did_set_texture_uniforms;
  }

  real32 curr_x = start_x;
  real32 curr_y = start_y;
  size_t str_length = strlen(str);
  size_t str_printable_length = 0;

  for (uint32 idx = 0; idx < str_length; idx++) {
    char c = str[idx];

    if (c < 32) {
      if (c == '\n') {
        curr_x = start_x;
        curr_y -= line_height;
      }
      continue;
    }

    Character *character = font_asset->characters.get(c);

    if (!character) {
      log_warning("Could not get character: %c", c);
      continue;
    }

    real32 char_x = curr_x + character->bearing.x * scale;
    real32 char_y = curr_y - (character->size.y - character->bearing.y) * scale;

    real32 tex_x = character->texture_x;
    real32 tex_w = (real32)character->size.x / font_asset->atlas_width;
    real32 tex_h = (real32)character->size.y / font_asset->atlas_height;

    real32 w = character->size.x * scale;
    real32 h = character->size.y * scale;

    curr_x += (character->advance.x >> 6) * scale;
    curr_y += (character->advance.y >> 6) * scale;

    // Skip glyphs with no pixels, like spaces.
    if (w <= 0 || h <= 0) {
      continue;
    }

    // NOTE: The correspondence between the y and texture y is the other way
    // around because the characters are upside down.
    real32 character_vertices[GUI_VERTEX_LENGTH * 6] = {
      char_x,     char_y + h,  tex_x,         0,     color.r, color.g, color.b, color.a,
      char_x,     char_y,      tex_x,         tex_h, color.r, color.g, color.b, color.a,
      char_x + w, char_y,      tex_x + tex_w, tex_h, color.r, color.g, color.b, color.a,
      char_x,     char_y + h,  tex_x,         0,     color.r, color.g, color.b, color.a,
      char_x + w, char_y,      tex_x + tex_w, tex_h, color.r, color.g, color.b, color.a,
      char_x + w, char_y + h,  tex_x + tex_w, 0,     color.r, color.g, color.b, color.a
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


GuiManager::GuiManager(
  Memory *memory, ShaderAsset *shader_asset
) :
  font_assets(
    Array<FontAsset>(
      &memory->asset_memory_pool, 8, "font_assets"
    )
  )
{
  this->memory = memory;
  this->shader_asset = shader_asset;

  // Shaders
  new(this->shader_asset) ShaderAsset(
    this->memory,
    "gui_element",
    SHADER_STANDARD,
    SHADER_DIR"gui_element.vert", SHADER_DIR"gui_element.frag", nullptr
  );

  // Fonts
  FT_Library ft_library;

  if (FT_Init_FreeType(&ft_library)) {
    log_error("Could not init FreeType");
    return;
  }

  new(this->font_assets.push()) FontAsset(
    this->memory, &ft_library, "main-font", DEFAULT_FONT, DEFAULT_FONT_SIZE
  );

  FT_Done_FreeType(ft_library);

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
