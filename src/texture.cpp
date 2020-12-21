const char* texture_type_to_string(TextureType texture_type) {
  if (texture_type == TextureType::none) {
    return "none";
  } else if (texture_type == TextureType::albedo) {
    return "albedo";
  } else if (texture_type == TextureType::metallic) {
    return "metallic";
  } else if (texture_type == TextureType::roughness) {
    return "roughness";
  } else if (texture_type == TextureType::ao) {
    return "ao";
  } else if (texture_type == TextureType::normal) {
    return "normal";
  } else if (texture_type == TextureType::shadowmap) {
    return "shadowmap";
  } else if (texture_type == TextureType::other) {
    return "other";
  } else if (texture_type == TextureType::g_position) {
    return "g_position";
  } else if (texture_type == TextureType::g_normal) {
    return "g_normal";
  } else if (texture_type == TextureType::g_albedo) {
    return "g_albedo";
  } else if (texture_type == TextureType::g_pbr) {
    return "g_pbr";
  } else if (texture_type == TextureType::l_color) {
    return "l_color";
  } else if (texture_type == TextureType::l_bright_color) {
    return "l_bright_color";
  } else if (texture_type == TextureType::l_depth) {
    return "l_depth";
  } else if (texture_type == TextureType::blur1) {
    return "blur1";
  } else if (texture_type == TextureType::blur2) {
    return "blur2";
  } else {
    log_error("Could not convert TextureType to string: %d", texture_type);
    return "<unknown>";
  }
}


TextureType texture_type_from_string(const char* str) {
  if (strcmp(str, "none") == 0) {
    return TextureType::none;
  } else if (strcmp(str, "albedo") == 0) {
    return TextureType::albedo;
  } else if (strcmp(str, "metallic") == 0) {
    return TextureType::metallic;
  } else if (strcmp(str, "roughness") == 0) {
    return TextureType::roughness;
  } else if (strcmp(str, "ao") == 0) {
    return TextureType::ao;
  } else if (strcmp(str, "normal") == 0) {
    return TextureType::normal;
  } else if (strcmp(str, "shadowmap") == 0) {
    return TextureType::shadowmap;
  } else if (strcmp(str, "other") == 0) {
    return TextureType::other;
  } else if (strcmp(str, "g_position") == 0) {
    return TextureType::g_position;
  } else if (strcmp(str, "g_normal") == 0) {
    return TextureType::g_normal;
  } else if (strcmp(str, "g_albedo") == 0) {
    return TextureType::g_albedo;
  } else if (strcmp(str, "g_pbr") == 0) {
    return TextureType::g_pbr;
  } else if (strcmp(str, "l_color") == 0) {
    return TextureType::l_color;
  } else if (strcmp(str, "l_bright_color") == 0) {
    return TextureType::l_bright_color;
  } else if (strcmp(str, "l_depth") == 0) {
    return TextureType::l_depth;
  } else if (strcmp(str, "blur1") == 0) {
    return TextureType::blur1;
  } else if (strcmp(str, "blur2") == 0) {
    return TextureType::blur2;
  } else {
    log_fatal("Could not parse TextureType from string: %s", str);
    return TextureType::none;
  }
}


bool32 Texture::is_type_screensize_dependent(TextureType type) {
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


Texture::Texture(
  TextureType type,
  const char* path
) :
  type(type)
{
  strcpy(this->path, TEXTURE_DIR);
  strcat(this->path, path);
  this->target = GL_TEXTURE_2D;
  this->is_screensize_dependent = Texture::is_type_screensize_dependent(type);
}


Texture::Texture(
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
