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

  // TODO: Remove `malloc()` and take Memory to use.
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

const char* util_join(char* buf, const char* prefix, uint32 n, const char* suffix) {
  assert(n < LEN(NUM_TO_STR));
  strcpy(buf, prefix);
  strcat(buf, NUM_TO_STR[n]);
  strcat(buf, suffix);
  return buf;
}

real64 util_random(real64 min, real64 max) {
  uint32 r = rand();
  real64 r_normalized = (real64)r / (real64)RAND_MAX;
  return min + ((r_normalized) * (max - min));
}
