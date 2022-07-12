// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include <assimp/cimport.h>
#include "../src_external/glad/glad.h"
#include "types.hpp"

class util {
public:
    static char const * stringify_glenum(GLenum thing);
    static GLenum get_texture_format_from_n_components(int32 n_components);
    static real64 random(real64 min, real64 max);
    static v3 aiVector3D_to_glm(aiVector3D *vec);
    static quat aiQuaternion_to_glm(aiQuaternion *rotation);
    static m4 aimatrix4x4_to_glm(aiMatrix4x4 *from);
    static void print_texture_internalformat_info(GLenum internal_format);
    static void APIENTRY debug_message_callback(
        GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
        char const *message, const void *userParam
    );
    static real32 round_to_nearest_multiple(real32 n, real32 multiple_of);
    static real64 get_us_from_duration(chrono::duration<real64> duration);
    static v3 get_orthogonal_vector(v3 *v);
    static uint32 kb_to_b(uint32 value);
    static uint32 mb_to_b(uint32 value);
    static uint32 gb_to_b(uint32 value);
    static uint32 tb_to_b(uint32 value);
    static real32 b_to_kb(uint32 value);
    static real32 b_to_mb(uint32 value);
    static real32 b_to_gb(uint32 value);
    static real32 b_to_tb(uint32 value);
};
