// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include <assimp/cimport.h>
#include "types.hpp"

class util {
public:
    static f64 random(f64 min, f64 max);
    static v3 aiVector3D_to_glm(aiVector3D *vec);
    static quat aiQuaternion_to_glm(aiQuaternion *rotation);
    static m4 aimatrix4x4_to_glm(aiMatrix4x4 *from);
    static f32 round_to_nearest_multiple(f32 n, f32 multiple_of);
    static f64 get_us_from_duration(chrono::duration<f64> duration);
    static v3 get_orthogonal_vector(v3 *v);
    static u32 kb_to_b(u32 value);
    static u32 mb_to_b(u32 value);
    static u32 gb_to_b(u32 value);
    static u32 tb_to_b(u32 value);
    static f32 b_to_kb(u32 value);
    static f32 b_to_mb(u32 value);
    static f32 b_to_gb(u32 value);
    static f32 b_to_tb(u32 value);
};
