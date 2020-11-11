constexpr real32 LINE_HEIGHT_FACTOR = 1.66f;
constexpr uint32 N_MAX_CHARACTERS_PER_DRAW = 4096;
constexpr const char *DEFAULT_FONT = "resources/fonts/iosevka-regular.ttf";
constexpr uint32 DEFAULT_FONT_SIZE = 18;


void TextManager::draw(
  const char* font_name, const char *str,
  real32 start_x, real32 start_y,
  real32 scale, glm::vec4 color
) {
  FontAsset *font_asset = FontAsset::get_by_name(&this->font_assets, font_name);

  uint16 line_height = (uint16)((real32)font_asset->font_size * LINE_HEIGHT_FACTOR);

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  glUseProgram(this->shader_asset->program);
  this->shader_asset->set_vec4("text_color", &color);
  this->shader_asset->set_mat4("text_projection", &this->text_projection);

  if (!this->shader_asset->did_set_texture_uniforms) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_asset->texture);
    this->shader_asset->did_set_texture_uniforms;
  }

  real32 curr_x = start_x;
  real32 curr_y = start_y;
  size_t str_length = strlen(str);
  size_t str_printable_length = 0;

  // TODO: It would be nice to only allocate as much as we need here.
  real32 vertices[6 * 4 * N_MAX_CHARACTERS_PER_DRAW];

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

    real32 char_texture_w = (real32)character->size.x / font_asset->atlas_width;
    real32 char_texture_h = (real32)character->size.y / font_asset->atlas_height;

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
    real32 character_vertices[6 * 4] = {
      char_x,     char_y + h,  character->texture_x,                  0,
      char_x,     char_y,      character->texture_x,                  char_texture_h,
      char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h,
      char_x,     char_y + h,  character->texture_x,                  0,
      char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h,
      char_x + w, char_y + h,  character->texture_x + char_texture_w, 0
    };
    memcpy(
      vertices + (6 * 4 * str_printable_length),
      character_vertices,
      sizeof(character_vertices)
    );
    str_printable_length += 1;
  }

  size_t vertices_size = sizeof(real32) * 6 * 4 * str_printable_length;
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices);
  glDrawArrays(GL_TRIANGLES, 0, 6 * (uint32)str_printable_length);
}


void TextManager::update_text_projection(
  uint32 window_width, uint32 window_height
) {
  this->text_projection = glm::ortho(
    0.0f, (real32)window_width, 0.0f, (real32)window_height
  );
}


TextManager::TextManager(
  Memory *memory, ShaderAsset *shader_asset,
  uint32 window_width, uint32 window_height
) :
  font_assets(
    Array<FontAsset>(
      &memory->asset_memory_pool, 8, "font_assets"
    )
  )
{
  this->shader_asset = shader_asset;

  update_text_projection(window_width, window_height);

  // Shaders
  new(this->shader_asset) ShaderAsset(
    memory,
    "text",
    SHADER_STANDARD,
    SHADER_DIR"text.vert", SHADER_DIR"text.frag", nullptr
  );

  // Fonts
  FT_Library ft_library;

  if (FT_Init_FreeType(&ft_library)) {
    log_error("Could not init FreeType");
    return;
  }

  new(this->font_assets.push()) FontAsset(
    memory, &ft_library, "main-font", DEFAULT_FONT, DEFAULT_FONT_SIZE
  );

  FT_Done_FreeType(ft_library);

  glGenVertexArrays(1, &this->vao);
  glGenBuffers(1, &this->vbo);
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * N_MAX_CHARACTERS_PER_DRAW,
    NULL, GL_DYNAMIC_DRAW
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
