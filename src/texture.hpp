#ifndef TEXTURE_HPP
#define TEXTURE_HPP

enum class TextureType {
  albedo,
  metallic,
  roughness,
  ao,
  normal,
  shadowmap,
  other,

  // Screensize-dependent textures
  g_position,
  g_normal,
  g_albedo,
  g_pbr,
  l_color,
  l_bright_color,
  l_depth,
  blur1,
  blur2
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
      type == TextureType::g_position ||
      type == TextureType::g_normal ||
      type == TextureType::g_albedo ||
      type == TextureType::g_pbr ||
      type == TextureType::l_color ||
      type == TextureType::l_bright_color ||
      type == TextureType::l_depth ||
      type == TextureType::blur1 ||
      type == TextureType::blur2
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
