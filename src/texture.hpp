#ifndef TEXTURE_H
#define TEXTURE_H

enum TextureType {
  TEXTURE_ALBEDO,
  TEXTURE_METALLIC,
  TEXTURE_ROUGHNESS,
  TEXTURE_AO,
  TEXTURE_NORMAL,
  TEXTURE_G_POSITION,
  TEXTURE_G_NORMAL,
  TEXTURE_G_ALBEDO,
  TEXTURE_G_PBR,
  TEXTURE_DEPTH,
  TEXTURE_OTHER
};

class Texture {
public:
  GLenum target;
  TextureType type;
  const char* uniform_name;
  const char* path;
  uint32 texture_name = 0;
  int32 width = 0;
  int32 height = 0;
  int32 n_components = 0;
  uint16 pbo_idx_for_copy;
  bool32 is_g_buffer_dependent = false;

  Texture(
    TextureType type,
    const char* uniform_name,
    const char* path
  ) :
    type(type),
    uniform_name(uniform_name),
    path(path)
  {
    this->target = GL_TEXTURE_2D;
    if (
      type == TEXTURE_G_POSITION ||
      type == TEXTURE_G_NORMAL ||
      type == TEXTURE_G_ALBEDO ||
      type == TEXTURE_G_PBR
    ) {
      this->is_g_buffer_dependent = true;
    }
  }

  Texture(
    GLenum target,
    TextureType type,
    const char* uniform_name,
    uint32 texture_name,
    int32 width,
    int32 height,
    int32 n_components
  ) :
    target(target),
    type(type),
    uniform_name(uniform_name),
    texture_name(texture_name),
    width(width),
    height(height),
    n_components(n_components)
  {
    if (
      type == TEXTURE_G_POSITION ||
      type == TEXTURE_G_NORMAL ||
      type == TEXTURE_G_ALBEDO ||
      type == TEXTURE_G_PBR
    ) {
      this->is_g_buffer_dependent = true;
    }
  }
};

#endif
