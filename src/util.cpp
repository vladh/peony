namespace Util {
  const char* join(char* buf, const char* s1, const char* s2, const char* s3) {
    strcpy(buf, s1);
    strcat(buf, s2);
    strcat(buf, s3);
    return buf;
  }

  const char* join(char* buf, const char* prefix, uint32 n, const char* suffix) {
    assert(n < LEN(NUM_TO_STR));
    return join(buf, prefix, NUM_TO_STR[n], suffix);
  }

  real64 random(real64 min, real64 max) {
    uint32 r = rand();
    real64 r_normalized = (real64)r / (real64)RAND_MAX;
    return min + ((r_normalized) * (max - min));
  }

  void sleep(real64 s) {
#if defined(__unix__)
    usleep(s * 1000 * 1000); // us
#endif
#if defined(_WIN32) || defined(_WIN64)
    Sleep((uint32)(s * 1000)); // ms
#endif
  }

  void make_sphere(
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
        real32 x_pos = cos(x_segment * 2.0f * PI32) * sin(y_segment * PI32);
        real32 y_pos = cos(y_segment * PI32);
        real32 z_pos = sin(x_segment * 2.0f * PI32) * sin(y_segment * PI32);

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

    for (uint32 y = 0; y < n_y_segments; y++) {
      if (y % 2 == 0) {
        /* for (int32 x = n_x_segments; x >= 0; x--) { */
        for (uint32 x = 0; x <= n_x_segments; x++) {
          index_data[idx_indices++] = (y + 1) * (n_x_segments + 1) + x;
          index_data[idx_indices++] = y * (n_x_segments + 1) + x;
          (*n_indices) += 2;
        }
      } else {
        /* for (uint32 x = 0; x <= n_x_segments; x++) { */
        for (int32 x = n_x_segments; x >= 0; x--) {
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
}
