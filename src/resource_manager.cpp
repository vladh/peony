uint32 ResourceManager::load_texture_from_file(const char *path, bool should_flip) {
  uint32 texture_id;
  glGenTextures(1, &texture_id);

  int32 width, height, n_components;
  unsigned char *data = load_image(
    path, &width, &height, &n_components, should_flip
  );

  if (data) {
    GLenum format = GL_RGB;
    if (n_components == 1) {
      format = GL_RED;
    } else if (n_components == 3) {
      format = GL_RGB;
    } else if (n_components == 4) {
      format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(
      GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free_image(data);
  } else {
    log_error("Texture failed to load at path: %s", path);
    free_image(data);
  }

  return texture_id;
}


uint32 ResourceManager::load_texture_from_file(const char *path) {
  return load_texture_from_file(path, true);
}


unsigned char* ResourceManager::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
) {
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


const char* ResourceManager::load_two_files(
  MemoryPool *pool, const char *path1, const char* path2
) {
  FILE* f1 = fopen(path1, "rb");
  if (!f1) {
    log_error("Could not open file %s.", path1);
    return nullptr;
  }
  fseek(f1, 0, SEEK_END);
  uint32 file1_size = ftell(f1);
  fseek(f1, 0, SEEK_SET);

  FILE* f2 = fopen(path2, "rb");
  if (!f2) {
    log_error("Could not open file %s.", path2);
    return nullptr;
  }
  fseek(f2, 0, SEEK_END);
  uint32 file2_size = ftell(f2);
  fseek(f2, 0, SEEK_SET);

  char *string = (char*)pool->push(file1_size + file2_size + 1);
  size_t result;

  result = fread(string, file1_size, 1, f1);
  fclose(f1);
  if (result != 1) {
    log_error("Could not read from file %s.", path1);
    return nullptr;
  }

  result = fread(string + file1_size, file2_size, 1, f2);
  fclose(f2);
  if (result != 1) {
    log_error("Could not read from file %s.", path2);
    return nullptr;
  }

  string[file1_size + file2_size] = 0;
  return string;
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

  char *string = (char*)pool->push(file_size + 1);
  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}
