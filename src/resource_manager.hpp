#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

namespace ResourceManager {
  uint32 load_texture_from_file(const char *path, bool should_flip);
  uint32 load_texture_from_file(const char *path);
  unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
  );
  unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels
  );
  void free_image(unsigned char *image_data);
  const char* load_two_files(MemoryPool *pool, const char *path1, const char* path2);
  const char* load_file(MemoryPool *pool, const char *path);
};

#endif
