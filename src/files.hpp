#pragma once

#include "types.hpp"

namespace files {
  unsigned char* load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
  );
  unsigned char* load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels
  );
  void free_image(unsigned char *image_data);
  uint32 get_file_size(const char *path);
  const char* load_file(MemoryPool *memory_pool, const char *path);
  const char* load_file(char *string, const char *path);
};
