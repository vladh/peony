/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include "../src_external/glad/glad.h"
#include "types.hpp"
#include "memory.hpp"
#include "constants.hpp"

class shaders {
public:
    enum class Type { none, standard, depth };

    struct Asset {
        char name[MAX_DEBUG_NAME_LENGTH];
        char vert_path[MAX_PATH];
        char frag_path[MAX_PATH];
        char geom_path[MAX_PATH];
        uint32 program;
        Type type;
        uint32 n_texture_units;
        uint32 texture_units[MAX_N_TEXTURE_UNITS];
        GLenum texture_unit_types[MAX_N_TEXTURE_UNITS];
        bool did_set_texture_uniforms;

        /*
           Intrinsic uniform: A uniform declared by a shader. We only care
           about intrinsic uniforms in the Shader struct.

           Active uniforms: A uniform used in a shader, which can be either
           an intrinsic uniform, or a uniform from another source such as a
           uniform buffer object.
       */

        uint32 n_intrinsic_uniforms;
        int32 intrinsic_uniform_locations[MAX_N_UNIFORMS];
        char intrinsic_uniform_names[MAX_UNIFORM_NAME_LENGTH][MAX_N_UNIFORMS];
    };

    static const char* shader_type_to_string(Type shader_type);
    static Type shader_type_from_string(const char* str);
    static bool is_shader_asset_valid(Asset *shader_asset);
    static void set_int(Asset *shader_asset, const char *uniform_name, uint32 value);
    static void set_bool(Asset *shader_asset, const char *uniform_name, bool value);
    static void set_float(Asset *shader_asset, const char *uniform_name, float value);
    static void set_vec2(Asset *shader_asset, const char *uniform_name, v2 *value);
    static void set_vec3(Asset *shader_asset, const char *uniform_name, v3 *value);
    static void set_vec4(Asset *shader_asset, const char *uniform_name, v4 *value);
    static void set_mat2(Asset *shader_asset, const char *uniform_name, m2 *mat);
    static void set_mat3(Asset *shader_asset, const char *uniform_name, m3 *mat);
    static void set_mat4_multiple(
        Asset *shader_asset, uint32 n, const char *uniform_name, m4 *mat
    );
    static void set_mat4(Asset *shader_asset, const char *uniform_name, m4 *mat);
    static void reset_texture_units(Asset *shader_asset);
    static uint32 add_texture_unit(
        Asset *shader_asset,
        uint32 new_texture_unit,
        GLenum new_texture_unit_type
    );
    static void load_shader_asset(Asset *shader_asset, MemoryPool *memory_pool);
    static Asset* init_shader_asset(
        Asset *shader_asset,
        MemoryPool *memory_pool,
        const char *new_name, Type new_type,
        const char *vert_path, const char *frag_path, const char *geom_path
    );
    static void destroy_shader_asset(Asset *shader_asset);

private:
    static void assert_shader_status_ok(uint32 new_shader, const char *path);
    static void assert_program_status_ok(uint32 new_program);
    static uint32 make_shader(const char *path, const char *source, GLenum shader_type);
    static uint32 make_program(uint32 vertex_shader, uint32 fragment_shader);
    static uint32 make_program(
        uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
    );
    static const char* load_file(MemoryPool *memory_pool, const char *path);
    static const char* load_frag_file(MemoryPool *memory_pool, const char *path);
    static int32 get_uniform_location(
        Asset *shader_asset,
        const char *uniform_name
    );
    static void load_uniforms(Asset *shader_asset);
};
