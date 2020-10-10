unsigned char* ResourceManager::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
) {
  // TODO: Change this to use our custom allocator.
  stbi_set_flip_vertically_on_load(should_flip);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  return image_data;
}


unsigned char* ResourceManager::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels
) {
  return load_image(path, width, height, n_channels, true);
}


void ResourceManager::free_image(unsigned char *image_data) {
  stbi_image_free(image_data);
}


uint32 ResourceManager::get_file_size(const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return 0;
  }
  fseek(f, 0, SEEK_END);
  uint32 size = ftell(f);
  fclose(f);
  return size;
}


const char* ResourceManager::load_file(MemoryPool *pool, const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char*)pool->push(file_size + 1, path);
  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}

const char* ResourceManager::load_file(char *string, const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}
