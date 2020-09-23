#ifndef UTIL_H
#define UTIL_H

unsigned char* util_load_image(const char *path, int32 *width, int32 *height, int32 *n_channels);
void util_free_image(unsigned char *image_data);
const char* util_load_two_files(MemoryPool *pool, const char *path1, const char* path2);
const char* util_load_file(MemoryPool *pool, const char *path);
const char* util_join(char* buf, const char* s1, const char* s2, const char* s3);
const char* util_join(char* buf, const char* prefix, uint32 n, const char* suffix);
real64 util_random(real64 min, real64 max);
void util_sleep(real64 s);
void util_make_sphere(
  uint32 n_x_segments, uint32 n_y_segments,
  uint32 *vertex_data_length, uint32 *index_data_length,
  uint32 *n_vertices, uint32 *n_indices,
  real32 *vertex_data, uint32 *index_data
);
glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from);
void log_aimatrix4x4(aiMatrix4x4* matrix);
void log_mat4(glm::mat4 *t);

#endif
