#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include <assimp/cimport.h>
#include "../src_external/glad/glad.h"
#include "types.hpp"

namespace util {
  const char* stringify_glenum(GLenum thing);
  GLenum get_texture_format_from_n_components(int32 n_components);
  real64 random(real64 min, real64 max);
  v3 aiVector3D_to_glm(aiVector3D *vec);
  quat aiQuaternion_to_glm(aiQuaternion *rotation);
  m4 aimatrix4x4_to_glm(aiMatrix4x4 *from);
  void print_texture_internalformat_info(GLenum internal_format);
  void APIENTRY debug_message_callback(
    GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
    const char *message, const void *userParam
  );
  real32 round_to_nearest_multiple(real32 n, real32 multiple_of);
  real64 get_us_from_duration(chrono::duration<real64> duration);
  v3 get_orthogonal_vector(v3 *v);
  uint32 kb_to_b(uint32 value);
  uint32 mb_to_b(uint32 value);
  uint32 gb_to_b(uint32 value);
  uint32 tb_to_b(uint32 value);
  real32 b_to_kb(uint32 value);
  real32 b_to_mb(uint32 value);
  real32 b_to_gb(uint32 value);
  real32 b_to_tb(uint32 value);
};
