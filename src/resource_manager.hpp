#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

class ResourceManager {
public:
  static uint32 load_texture_from_image_data_and_free(
    unsigned char *data, int32 *width, int32 *height, int32 *n_components
  );
  static unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
  );
  static unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels
  );
  static void free_image(unsigned char *image_data);
  static const char* load_two_files(MemoryPool *pool, const char *path1, const char* path2);
  static const char* load_file(MemoryPool *pool, const char *path);
};

#endif
