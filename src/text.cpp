void text_draw(
  State *state, const char* font_name, const char *str,
  real32 x, real32 y, real32 scale, glm::vec4 color
) {
  ShaderAsset *shader_asset = state->text_shader_asset;
  FontAsset *font_asset = FontAsset::get_by_name(&state->font_assets, font_name);

  glUseProgram(shader_asset->program);
  shader_asset->set_vec4("text_color", &color);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font_asset->texture);

  glBindVertexArray(state->text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);

  for (uint32 idx = 0; idx < strlen(str); idx++) {
    char c = str[idx];
    Character *character = font_asset->characters.get(c);

    real32 char_x = x + character->bearing.x * scale;
    real32 char_y = y - (character->size.y - character->bearing.y) * scale;

    real32 char_texture_w = (real32)character->size.x / font_asset->atlas_width;
    real32 char_texture_h = (real32)character->size.y / font_asset->atlas_height;

    real32 w = character->size.x * scale;
    real32 h = character->size.y * scale;

    x += (character->advance.x >> 6) * scale;
    y += (character->advance.y >> 6) * scale;

    if (w == 0 || h == 0) {
      // Skip glyphs with no pixels, like spaces.
      continue;
    }

    // TODO: Buffer vertices only once, use a matrix to transform the position.
    // NOTE: The correspondence between the y and texture y is the other way
    // around because the characters are upside down for some reason.
    real32 vertices[6][4] = {
      {char_x,     char_y + h,  character->texture_x,                  0},
      {char_x,     char_y,      character->texture_x,                  char_texture_h},
      {char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h},
      {char_x,     char_y + h,  character->texture_x,                  0},
      {char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h},
      {char_x + w, char_y + h,  character->texture_x + char_texture_w, 0}
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}


void text_init(Memory *memory, State *state) {
  // Shaders
  ShaderAsset *text_shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "text",
    SHADER_UI,
    SHADER_DIR"text.vert", SHADER_DIR"text.frag"
  );
  glm::mat4 text_projection = glm::ortho(
    0.0f, (real32)state->window_width, 0.0f, (real32)state->window_height
  );
  glUseProgram(text_shader_asset->program);
  text_shader_asset->set_mat4("text_projection", &text_projection);
  state->text_shader_asset = text_shader_asset;

  // Fonts
  FT_Library ft_library;

  if (FT_Init_FreeType(&ft_library)) {
    log_error("Could not init FreeType");
    return;
  }

  new(state->font_assets.push()) FontAsset(
    memory,
    &ft_library,
    "main-font",
    /* "resources/fonts/AlrightSans-Regular.otf" */
    "resources/fonts/iosevka-regular.ttf"
  );

  FT_Done_FreeType(ft_library);

  glGenVertexArrays(1, &state->text_vao);
  glGenBuffers(1, &state->text_vbo);
  glBindVertexArray(state->text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
