#ifndef TEXTURE_NAME_POOL_HPP
#define TEXTURE_NAME_POOL_HPP

constexpr uint32 N_MAX_TEXTURE_POOL_SIZES = 6;

class TextureNamePool {
public:
  TextureNamePool(
    Memory *memory,
    uint32 n_textures,
    uint32 mipmap_max_level
  );
  void allocate_texture_names();
  uint32 get_next(uint32 target_size);

  uint32 mipmap_max_level;
  uint32 n_textures;
  uint32 n_sizes;
  uint32 sizes[N_MAX_TEXTURE_POOL_SIZES];

private:
  uint32 idx_next[N_MAX_TEXTURE_POOL_SIZES] = {0};
  uint32 *texture_names;
};

#endif
