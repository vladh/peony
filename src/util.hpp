#ifndef UTIL_H
#define UTIL_H

class Util {
public:
  static const char* join(char* buf, const char* s1, const char* s2, const char* s3);
  static const char* join(char* buf, const char* prefix, uint32 n, const char* suffix);
  static real64 random(real64 min, real64 max);
  static void sleep(real64 s);
  static void make_sphere(
    uint32 n_x_segments, uint32 n_y_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 *vertex_data, uint32 *index_data
  );
  static glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from);
};

#endif
