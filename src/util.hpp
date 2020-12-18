#ifndef UTIL_H
#define UTIL_H

class Util {
public:
  static const char* stringify_glenum(GLenum thing);
  static GLenum get_texture_format_from_n_components(int32 n_components);
  static real64 random(real64 min, real64 max);
  static void sleep(real64 s);
  static void make_pointcloud(
    MemoryPool *memory_pool,
    uint32 x_size, uint32 y_size, uint32 z_size,
    uint32 n_x_segments, uint32 n_y_segments, uint32 n_z_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 **vertex_data, uint32 **index_data
  );
  static void make_plane(
    MemoryPool *memory_pool,
    uint32 x_size, uint32 z_size,
    uint32 n_x_segments, uint32 n_z_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 **vertex_data, uint32 **index_data
  );
  static void make_sphere(
    MemoryPool *memory_pool,
    uint32 n_x_segments, uint32 n_y_segments,
    uint32 *n_vertices, uint32 *n_indices,
    real32 **vertex_data, uint32 **index_data
  );
  static glm::mat4 aimatrix4x4_to_glm(aiMatrix4x4* from);
  static void print_texture_internalformat_info(GLenum internal_format);
  static void APIENTRY debug_message_callback(
    GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
    const char *message, const void *userParam
  );
  static real32 round_to_nearest_multiple(real32 n, real32 multiple_of);
};

#endif
