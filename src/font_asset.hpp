#ifndef FONTS_H
#define FONTS_H

#define N_CHARS_TO_LOAD 128

struct Character {
  glm::ivec2 size;
  glm::ivec2 bearing;
  glm::ivec2 advance;
  real32 texture_x;
};

class FontAsset {
public:
  const char *name;
  Array<Character> characters;
  uint32 atlas_width;
  uint32 atlas_height;
  uint32 texture;
  uint32 font_size;

  FontAsset(
    Memory *memory, FT_Library *ft_library,
    const char *name, const char *path,
    uint16 font_size
  );
  void load_glyphs(FT_Face face);
  static FontAsset* get_by_name(
    Array<FontAsset> *assets, const char *name
  );
};

#endif
