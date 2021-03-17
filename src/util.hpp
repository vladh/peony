#ifndef UTIL_HPP
#define UTIL_HPP

namespace Util {
  unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
  );
  unsigned char *load_image(
    const char *path, int32 *width, int32 *height, int32 *n_channels
  );
  void free_image(unsigned char *image_data);
  const char* load_file(MemoryPool *pool, const char *path);
  const char* load_file(char *string, const char *path);
  uint32 get_file_size(const char *path);
  const char* stringify_glenum(GLenum thing);
  GLenum get_texture_format_from_n_components(int32 n_components);
  real64 random(real64 min, real64 max);
  void sleep(real64 s);
  void make_plane(
    MemoryPool *memory_pool,
    uint32 x_size, uint32 z_size,
    uint32 n_x_segments, uint32 n_z_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 **vertex_data, uint32 **index_data
  );
  void make_sphere(
    MemoryPool *memory_pool,
    uint32 n_x_segments, uint32 n_y_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 **vertex_data, uint32 **index_data
  );
  glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from);
  void print_texture_internalformat_info(GLenum internal_format);
  void APIENTRY debug_message_callback(
    GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
    const char *message, const void *userParam
  );
  real32 round_to_nearest_multiple(real32 n, real32 multiple_of);
  real64 get_us_from_duration(chrono::duration<real64> duration);
};

#endif
