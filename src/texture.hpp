#ifndef TEXTURE_H
#define TEXTURE_H

enum TextureType {
  TEXTURE_ALBEDO,
  TEXTURE_METALLIC,
  TEXTURE_ROUGHNESS,
  TEXTURE_AO,
  TEXTURE_NORMAL,
  TEXTURE_OTHER
};

class Texture {
public:
  TextureType type;
  const char* uniform_name;
  const char* path;
  uint32 texture_name;
  int32 width;
  int32 height;
  int32 n_components;
  uint16 pbo_idx_for_copy;

  Texture(
    TextureType type,
    const char* uniform_name,
    const char* path
  ) :
    type(type),
    uniform_name(uniform_name),
    path(path)
  {
  }

  Texture(
    TextureType type,
    const char* uniform_name,
    uint32 texture_name,
    int32 width,
    int32 height,
    int32 n_components
  ) :
    type(type),
    uniform_name(uniform_name),
    texture_name(texture_name),
    width(width),
    height(height),
    n_components(n_components)
  {
  }
};

#endif
