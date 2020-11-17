#ifndef FONTS_H
#define FONTS_H

constexpr uint32 N_CHARS_TO_LOAD = 128;

struct Character {
  glm::ivec2 size;
  glm::ivec2 bearing;
  glm::ivec2 advance;
  glm::ivec2 tex_coords;
};

class FontAsset {
public:
  const char *name;
  Array<Character> characters;
  uint32 texture;
  uint32 font_size;
  uint32 units_per_em;
  uint32 ascender;
  uint32 descender;
  uint32 height;

  FontAsset(
    Memory *memory,
    TextureAtlas *texture_atlas,
    FT_Library *ft_library,
    const char *name,
    const char *path,
    uint16 font_size
  );
  real32 frac_px_to_px(uint32 n);
  real32 font_unit_to_px(uint32 n);
  void load_glyphs(
    FT_Face face, TextureAtlas *texture_atlas
  );
  static FontAsset* get_by_name(
    Array<FontAsset> *assets, const char *name
  );
};

#endif
