#ifndef FONTS_H
#define FONTS_H

struct FontAsset;

struct Character {
  uint32 texture;
  glm::ivec2 size;
  glm::ivec2 bearing;
  uint32 advance;
};

struct Font {
  Array<Character> characters;
};

void font_load_glyphs(
  FontAsset *asset, FT_Face *face
);

FontAsset* font_make_asset(
  FontAsset *asset, FT_Library *ft_library,
  const char *name, const char *path
);

#endif
