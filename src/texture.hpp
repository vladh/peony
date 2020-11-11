#ifndef TEXTURE_H
#define TEXTURE_H

enum TextureType {
  TEXTURE_ALBEDO,
  TEXTURE_METALLIC,
  TEXTURE_ROUGHNESS,
  TEXTURE_AO,
  TEXTURE_NORMAL,
  TEXTURE_SHADOWMAP,
  TEXTURE_OTHER,

  // Screensize-dependent textures
  TEXTURE_G_POSITION,
  TEXTURE_G_NORMAL,
  TEXTURE_G_ALBEDO,
  TEXTURE_G_PBR,
  TEXTURE_L_COLOR,
  TEXTURE_L_BRIGHT_COLOR,
  TEXTURE_L_DEPTH,
  TEXTURE_BLUR1,
  TEXTURE_BLUR2
};

class Texture {
public:
  GLenum target;
  TextureType type;
  const char* path;
  uint32 texture_name = 0;
  int32 width = 0;
  int32 height = 0;
  int32 n_components = 0;
  uint16 pbo_idx_for_copy;
  bool32 is_screensize_dependent = false;

  static bool32 is_type_screensize_dependent(TextureType type) {
    return (
      type == TEXTURE_G_POSITION ||
      type == TEXTURE_G_NORMAL ||
      type == TEXTURE_G_ALBEDO ||
      type == TEXTURE_G_PBR ||
      type == TEXTURE_L_COLOR ||
      type == TEXTURE_L_BRIGHT_COLOR ||
      type == TEXTURE_L_DEPTH ||
      type == TEXTURE_BLUR1 ||
      type == TEXTURE_BLUR2
    );
  }

  Texture(
    TextureType type,
    const char* path
  ) :
    type(type),
    path(path)
  {
    this->target = GL_TEXTURE_2D;
    this->is_screensize_dependent = Texture::is_type_screensize_dependent(type);
  }

  Texture(
    GLenum target,
    TextureType type,
    uint32 texture_name,
    int32 width,
    int32 height,
    int32 n_components
  ) :
    target(target),
    type(type),
    texture_name(texture_name),
    width(width),
    height(height),
    n_components(n_components)
  {
    this->is_screensize_dependent = Texture::is_type_screensize_dependent(type);
  }
};

#endif
