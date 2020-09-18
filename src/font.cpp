void font_load_glyphs(
  FontAsset *asset, FT_Face *face
) {
  for (unsigned char c = 0; c < 128; c++) {
    Character *character = array_push(&asset->font.characters);

    if (FT_Load_Char(*face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    FT_GlyphSlot glyph = (*face)->glyph;

    glGenTextures(1, &character->texture);
    glBindTexture(GL_TEXTURE_2D, character->texture);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RED,
      glyph->bitmap.width,
      glyph->bitmap.rows,
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      glyph->bitmap.buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    character->size = glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows);
    character->bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
    character->advance = glyph->advance.x;
  }
}

FontAsset* font_make_asset(
  Memory *memory, FontAsset *asset, FT_Library *ft_library,
  const char *name, const char *path
) {
  asset->info.name = name;

  array_init<Character>(
    &memory->asset_memory_pool, &asset->font.characters, 128
  );

  FT_Face face;

  if (FT_New_Face(*ft_library, path, 0, &face)) {
    log_error("Could not load font at %s", path);
    return nullptr;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  font_load_glyphs(asset, &face);
  // TODO: Do we want to set this back?
  /* glPixelStorei(GL_UNPACK_ALIGNMENT, 4); */

  FT_Done_Face(face);

  return asset;
}
