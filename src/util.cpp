#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "types.hpp"
#include "log.hpp"


unsigned char *util_load_image(const char *path, int32 *width, int32 *height, int32 *n_channels) {
  stbi_set_flip_vertically_on_load(true);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  return image_data;
}

void util_free_image(unsigned char *image_data) {
  stbi_image_free(image_data);
}

char* util_load_file(const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char*)malloc(fsize + 1);
  size_t result = fread(string, fsize, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[fsize] = 0;

  return string;
}
