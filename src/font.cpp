void font_load_glyphs(
  FontAsset *asset, FT_Face face
) {
  Font *font = &asset->font;
  FT_GlyphSlot glyph = face->glyph;

  font->atlas_width = 0;
  font->atlas_height = 0;

  // TODO: Can we avoid loading all characters twice here?
  for (unsigned char c = 0; c < N_CHARS_TO_LOAD; c++) {
    Character *character = array_push(&font->characters);

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    character->size = glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows);
    character->bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
    character->advance = glm::ivec2(glyph->advance.x, glyph->advance.y);

    font->atlas_width += glyph->bitmap.width;
    font->atlas_height = MAX(font->atlas_height, glyph->bitmap.rows);
  }

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &font->texture);
  glBindTexture(GL_TEXTURE_2D, font->texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RED,
    font->atlas_width, font->atlas_height,
    0, GL_RED, GL_UNSIGNED_BYTE, 0
  );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  uint32 texture_x = 0;

  for (unsigned char c = 0; c < N_CHARS_TO_LOAD; c++) {
    Character *character = &font->characters.items[c];

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    glTexSubImage2D(
      GL_TEXTURE_2D, 0, texture_x, 0,
      glyph->bitmap.width, glyph->bitmap.rows,
      GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer
    );

    character->texture_x = (real32)texture_x / font->atlas_width;
    texture_x += glyph->bitmap.width;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

FontAsset* font_make_asset(
  Memory *memory, FontAsset *asset, FT_Library *ft_library,
  const char *name, const char *path
) {
  asset->info.name = name;

  array_init<Character>(
    &memory->asset_memory_pool, &asset->font.characters, N_CHARS_TO_LOAD
  );

  FT_Face face;

  if (FT_New_Face(*ft_library, path, 0, &face)) {
    log_error("Could not load font at %s", path);
    return nullptr;
  }

  // TODO: Support multiple sizes somehow.
  FT_Set_Pixel_Sizes(face, 0, 48);

  font_load_glyphs(asset, face);

  FT_Done_Face(face);

  return asset;
}
