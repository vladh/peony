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
  type(type),
  path(path)
{
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
