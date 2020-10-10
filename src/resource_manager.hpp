#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

class ResourceManager {
public:
  static unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
  );
  static unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels
  );
  static void free_image(unsigned char *image_data);
  static const char* load_file(MemoryPool *pool, const char *path);
  static const char* load_file(char *string, const char *path);
  static uint32 get_file_size(const char *path);
};

#endif
