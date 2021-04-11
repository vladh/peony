real32 Fonts::frac_px_to_px(uint32 n) {
  return (real32)(n >> 6);
}


real32 Fonts::font_unit_to_px(uint32 n) {
  // NOTE: We should be dividing by units_per_em here...probably?
  // This is because we expect height etc. to be in "font units".
  // But treating these metrics as "fractional pixels" seems to work,
  // whereas division by units_per_em doesn't.
  // Check this in more detail.
  return (real32)(n >> 6);
}


FontAsset* Fonts::get_by_name(
  Array<FontAsset> *assets, const char *name
) {
  for_each (asset, *assets) {
    if (Str::eq(asset->name, name)) {
      return asset;
    }
  }
  log_warning("Could not find FontAsset with name %s", name);
  return nullptr;
}


void Fonts::load_glyphs(
  FontAsset *font_asset,
  FT_Face face,
  TextureAtlas *texture_atlas
) {
  FT_GlyphSlot glyph = face->glyph;

  // TODO: Can we avoid loading all characters twice here?
  for (uint32 c = 0; c < CHAR_MAX_CODEPOINT_TO_LOAD; c++) {
    Character *character = font_asset->characters.push();

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    character->size = glm::ivec2(glyph->bitmap.width, glyph->bitmap.rows);
    character->bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
    character->advance = glm::ivec2(glyph->advance.x, glyph->advance.y);
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_atlas->texture_name);

  for (uint32 c = 0; c < CHAR_MAX_CODEPOINT_TO_LOAD; c++) {
    if (
      // Unicode C0 controls
      (c >= 0x00 && c <= 0x1F) ||
      // DEL
      (c == 0x7F) ||
      // Unicode C1 controls
      (c >= 0x80 && c <= 0x9F)
    ) {
      continue;
    }

    Character *character = font_asset->characters[c];

    glm::ivec2 tex_coords = Materials::push_space_to_texture_atlas(
      texture_atlas, character->size
    );

    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      log_error("Failed to load glyph %s", c);
      continue;
    }

    glTexSubImage2D(
      GL_TEXTURE_2D, 0, tex_coords.x, tex_coords.y,
      character->size.x, character->size.y,
      GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer
    );

    character->tex_coords = tex_coords;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}


FontAsset* Fonts::init_font_asset(
  FontAsset *font_asset,
  MemoryPool *memory_pool,
  TextureAtlas *texture_atlas,
  FT_Library *ft_library,
  const char *name,
  const char *path,
  uint16 font_size
) {
  font_asset->name = name;
  font_asset->font_size = font_size;

  font_asset->characters = Array<Character>(
    memory_pool,
    CHAR_MAX_CODEPOINT_TO_LOAD + 1,
    "characters"
  );

  FT_Face face;
  if (FT_New_Face(*ft_library, path, 0, &face)) {
    log_error("Could not load font at %s", path);
  }
  FT_Set_Pixel_Sizes(face, 0, font_asset->font_size);
  if (!FT_IS_SCALABLE(face)) {
    log_fatal("Font face not scalable, don't know what to do.");
  }
  font_asset->units_per_em = face->units_per_EM;
  font_asset->ascender = face->ascender;
  font_asset->descender = face->descender;
  font_asset->height = face->height;

  load_glyphs(font_asset, face, texture_atlas);

  FT_Done_Face(face);

  return font_asset;
}
