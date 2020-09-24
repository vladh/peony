unsigned char *util_load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
) {
  stbi_set_flip_vertically_on_load(should_flip);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  return image_data;
}

unsigned char *util_load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels
) {
  util_load_image(path, width, height, n_channels, true);
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

void util_make_sphere(
  uint32 n_x_segments, uint32 n_y_segments,
  uint32 *n_vertices, uint32 *n_indices,
  real32 *vertex_data, uint32 *index_data
) {
  uint32 idx_vertices = 0;
  uint32 idx_indices = 0;
  *n_vertices = 0;
  *n_indices = 0;

  for (uint32 y = 0; y <= n_y_segments; y++) {
    for (uint32 x = 0; x <= n_x_segments; x++) {
      real32 x_segment = (real32)x / (real32)n_x_segments;
      real32 y_segment = (real32)y / (real32)n_y_segments;
      real32 x_pos = cos(x_segment * 2.0f * (real32)PI) * sin(y_segment * (real32)PI);
      real32 y_pos = cos(y_segment * (real32)PI);
      real32 z_pos = sin(x_segment * 2.0f * (real32)PI) * sin(y_segment * (real32)PI);

      // Position
      vertex_data[idx_vertices++] = x_pos;
      vertex_data[idx_vertices++] = y_pos;
      vertex_data[idx_vertices++] = z_pos;
      // Normal
      vertex_data[idx_vertices++] = x_pos;
      vertex_data[idx_vertices++] = y_pos;
      vertex_data[idx_vertices++] = z_pos;
      // Tex coords
      vertex_data[idx_vertices++] = x_segment;
      vertex_data[idx_vertices++] = y_segment;

      (*n_vertices)++;
    }
  }

  for (uint32 y = 0; y < n_y_segments; ++y) {
    if (y % 2 == 0) {
      for (int32 x = n_x_segments; x >= 0; x--) {
        index_data[idx_indices++] = (y + 1) * (n_x_segments + 1) + x;
        index_data[idx_indices++] = y * (n_x_segments + 1) + x;
        (*n_indices) += 2;
      }
    } else {
      for (uint32 x = 0; x <= n_x_segments; x++) {
        index_data[idx_indices++] = y * (n_x_segments + 1) + x;
        index_data[idx_indices++] = (y + 1) * (n_x_segments + 1) + x;
        (*n_indices) += 2;
      }
    }
  }
}

glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from) {
  glm::mat4 to;

  to[0][0] = (GLfloat)from->a1;
  to[0][1] = (GLfloat)from->b1;
  to[0][2] = (GLfloat)from->c1;
  to[0][3] = (GLfloat)from->d1;

  to[1][0] = (GLfloat)from->a2;
  to[1][1] = (GLfloat)from->b2;
  to[1][2] = (GLfloat)from->c2;
  to[1][3] = (GLfloat)from->d2;

  to[2][0] = (GLfloat)from->a3;
  to[2][1] = (GLfloat)from->b3;
  to[2][2] = (GLfloat)from->c3;
  to[2][3] = (GLfloat)from->d3;

  to[3][0] = (GLfloat)from->a4;
  to[3][1] = (GLfloat)from->b4;
  to[3][2] = (GLfloat)from->c4;
  to[3][3] = (GLfloat)from->d4;

  return to;
}

void log_aimatrix4x4(aiMatrix4x4 *t) {
  log_info("(%f, %f, %f, %f)", t->a1, t->b1, t->c1, t->d1);
  log_info("(%f, %f, %f, %f)", t->a2, t->b2, t->c2, t->d2);
  log_info("(%f, %f, %f, %f)", t->a3, t->b3, t->c3, t->d3);
  log_info("(%f, %f, %f, %f)", t->a4, t->b4, t->c4, t->d4);
}

void log_mat4(glm::mat4 *t) {
  log_info("(%f, %f, %f, %f)", (*t)[0][0], (*t)[1][0], (*t)[2][0], (*t)[3][0]);
  log_info("(%f, %f, %f, %f)", (*t)[0][1], (*t)[1][1], (*t)[2][1], (*t)[3][1]);
  log_info("(%f, %f, %f, %f)", (*t)[0][2], (*t)[1][2], (*t)[2][2], (*t)[3][2]);
  log_info("(%f, %f, %f, %f)", (*t)[0][3], (*t)[1][3], (*t)[2][3], (*t)[3][3]);
}
