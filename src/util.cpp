unsigned char *util_load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels
) {
  stbi_set_flip_vertically_on_load(true);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  return image_data;
}

void util_free_image(unsigned char *image_data) {
  stbi_image_free(image_data);
}

const char* util_load_two_files(MemoryPool *pool, const char *path1, const char* path2) {
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

  char *string = (char*)memory_push_memory_to_pool(pool, file1_size + file2_size + 1);
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

const char* util_load_file(MemoryPool *pool, const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char*)memory_push_memory_to_pool(pool, file_size + 1);
  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}

const char* util_join(char* buf, const char* s1, const char* s2, const char* s3) {
  strcpy(buf, s1);
  strcat(buf, s2);
  strcat(buf, s3);
  return buf;
}

const char* util_join(char* buf, const char* prefix, uint32 n, const char* suffix) {
  assert(n < LEN(NUM_TO_STR));
  return util_join(buf, prefix, NUM_TO_STR[n], suffix);
}

real64 util_random(real64 min, real64 max) {
  uint32 r = rand();
  real64 r_normalized = (real64)r / (real64)RAND_MAX;
  return min + ((r_normalized) * (max - min));
}

void util_sleep(real64 s) {
#if defined(__unix__)
  usleep(s * 1000 * 1000); // us
#endif
#if defined(_WIN32) || defined(_WIN64)
  Sleep((uint32)(s * 1000)); // ms
#endif
}
