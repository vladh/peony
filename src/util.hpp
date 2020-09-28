#ifndef UTIL_H
#define UTIL_H

namespace Util {
  const char* join(char* buf, const char* s1, const char* s2, const char* s3);
  const char* join(char* buf, const char* prefix, uint32 n, const char* suffix);
  real64 random(real64 min, real64 max);
  void sleep(real64 s);
  void make_sphere(
    uint32 n_x_segments, uint32 n_y_segments,
    uint32 *vertex_data_length, uint32 *index_data_length,
    uint32 *n_vertices, uint32 *n_indices,
    real32 *vertex_data, uint32 *index_data
  );
  glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from);
}

#endif
