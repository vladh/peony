#ifndef TEXTURE_POOL_H
#define TEXTURE_POOL_H

class TextureNamePool {
public:
  TextureNamePool(
    Memory *memory,
    uint32 n_textures,
    uint32 texture_width, uint32 texture_height, uint32 texture_depth,
    uint32 mipmap_max_level
  );
  void allocate_texture_names();
  uint32 get_next();

  uint32 texture_width;
  uint32 texture_height;
  uint32 texture_depth;
  uint32 mipmap_max_level;
  uint32 n_textures;

private:
  uint32 idx_next = 0;
  uint32 *texture_names;
};

#endif
