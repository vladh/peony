// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "memory.hpp"
#include "stb.hpp"
#include "files.hpp"


unsigned char* files::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
) {
  stbi_set_flip_vertically_on_load(should_flip);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  if (!image_data) {
    logs::fatal("Could not open file %s.", path);
  }
  return image_data;
}


unsigned char* files::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels
) {
  return load_image(path, width, height, n_channels, true);
}


void files::free_image(unsigned char *image_data) {
  stbi_image_free(image_data);
}


uint32 files::get_file_size(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    logs::error("Could not open file %s.", path);
    return 0;
  }
  fseek(f, 0, SEEK_END);
  uint32 size = ftell(f);
  fclose(f);
  return size;
}


const char* files::load_file(MemoryPool *memory_pool, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    logs::error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char*)memory::push(memory_pool, file_size + 1, path);
  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    logs::error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}


const char* files::load_file(char *string, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    logs::error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    logs::error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}
