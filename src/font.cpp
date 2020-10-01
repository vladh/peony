void FontAsset::load_glyphs(FT_Face face) {
  FT_GlyphSlot glyph = face->glyph;

  this->atlas_width = 0;
  this->atlas_height = 0;

  // TODO: Can we avoid loading all characters twice here?
  for (unsigned char c = 0; c < N_CHARS_TO_LOAD; c++) {
    Character *character = this->characters.push();

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    character->size = glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows);
    character->bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
    character->advance = glm::ivec2(glyph->advance.x, glyph->advance.y);

    this->atlas_width += glyph->bitmap.width;
    this->atlas_height = MAX(this->atlas_height, glyph->bitmap.rows);
  }

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RED,
    this->atlas_width, this->atlas_height,
    0, GL_RED, GL_UNSIGNED_BYTE, 0
  );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  uint32 texture_x = 0;

  for (unsigned char c = 0; c < N_CHARS_TO_LOAD; c++) {
    Character *character = this->characters.get(c);

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    glTexSubImage2D(
      GL_TEXTURE_2D, 0, texture_x, 0,
      glyph->bitmap.width, glyph->bitmap.rows,
      GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer
    );

    character->texture_x = (real32)texture_x / this->atlas_width;
    texture_x += glyph->bitmap.width;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}


FontAsset::FontAsset(
  Memory *memory, FT_Library *ft_library,
  const char *name, const char *path
) : characters(&memory->asset_memory_pool, N_CHARS_TO_LOAD)
{
  this->name = name;

  FT_Face face;

  if (FT_New_Face(*ft_library, path, 0, &face)) {
    log_error("Could not load font at %s", path);
  }

  // TODO: Support multiple sizes somehow.
  FT_Set_Pixel_Sizes(face, 0, 18);

  load_glyphs(face);

  FT_Done_Face(face);
}


FontAsset* FontAsset::get_by_name(
  Array<FontAsset> *assets, const char *name
) {
  for (uint32 idx = 0; idx < assets->get_size(); idx++) {
    FontAsset *asset = assets->get(idx);
    if (strcmp(asset->name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find FontAsset with name %s", name);
  return nullptr;
}
