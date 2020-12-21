#ifndef TEXTURE_HPP
#define TEXTURE_HPP

constexpr char TEXTURE_DIR[] = "resources/textures/";

enum class TextureType {
  none,
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

const char* texture_type_to_string(TextureType texture_type);
TextureType texture_type_from_string(const char* str);

class Texture {
public:
  GLenum target;
  TextureType type;
  char path[256]; // TODO: Fix unsafe strings?
  uint32 texture_name = 0;
  int32 width = 0;
  int32 height = 0;
  int32 n_components = 0;
  uint16 pbo_idx_for_copy;
  bool32 is_screensize_dependent = false;

  static bool32 is_type_screensize_dependent(TextureType type);
  Texture(
    TextureType type,
    const char* path
  );
  Texture(
    GLenum target,
    TextureType type,
    uint32 texture_name,
    int32 width,
    int32 height,
    int32 n_components
  );
};

#endif
