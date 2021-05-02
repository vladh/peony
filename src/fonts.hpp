#ifndef FONTS_HPP
#define FONTS_HPP

struct Character {
  glm::ivec2 size;
  glm::ivec2 bearing;
  glm::ivec2 advance;
  glm::ivec2 tex_coords;
};

struct FontAsset {
  const char *name;
  Array<Character> characters;
  uint32 texture;
  uint32 font_size;
  uint32 units_per_em;
  uint32 ascender;
  uint32 descender;
  uint32 height;
};

namespace Fonts {
  // NOTE: Unicode is only the same as ASCII until 0x7F.
  // We can change this to 0xFF when we add Unicode support.
  constexpr uint32 CHAR_MAX_CODEPOINT_TO_LOAD = 0x7F;

  real32 frac_px_to_px(uint32 n);
  real32 font_unit_to_px(uint32 n);
  FontAsset* get_by_name(
    Array<FontAsset> *assets, const char *name
  );
  void load_glyphs(
    FontAsset *font_asset,
    FT_Face face,
    TextureAtlas *texture_atlas
  );
  FontAsset* init_font_asset(
    FontAsset *font_asset,
    MemoryPool *memory_pool,
    TextureAtlas *texture_atlas,
    FT_Library *ft_library,
    const char *name,
    const char *filename,
    uint16 font_size
  );
}

#endif
