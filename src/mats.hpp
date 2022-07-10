// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "../src_external/glad/glad.h"
#include "memory.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "constants.hpp"
#include "shaders.hpp"
#include "array.hpp"
#include "types.hpp"

class mats {
public:
    struct TextureNamePool {
        uint32 mipmap_max_level;
        uint32 n_textures;
        uint32 n_sizes;
        uint32 sizes[MAX_N_TEXTURE_POOL_SIZES];
        uint32 idx_next[MAX_N_TEXTURE_POOL_SIZES];
        uint32 *texture_names;
    };

    struct PersistentPbo {
        uint32 pbo;
        void *memory;
        int32 width;
        int32 height;
        int32 n_components;
        uint16 texture_count;
        uint32 texture_size;
        uint32 total_size;
        uint16 next_idx;
    };

    struct TextureAtlas {
        iv2 size;
        iv2 next_position;
        iv2 max_allocated_position_per_axis;
        uint32 texture_name;
    };

    enum class TextureType {
        none,
        albedo,
        metallic,
        roughness,
        ao,
        normal,
        shadowmaps_3d,
        shadowmaps_2d,
        other,

        // Screensize-dependent textures
        g_position,
        g_normal,
        g_albedo,
        g_pbr,
        l_color,
        l_bright_color,
        l_depth,
        blur1,
        blur2
    };

    struct Texture {
        GLenum target;
        TextureType type;
        char path[MAX_PATH];
        uint32 texture_name;
        int32 width;
        int32 height;
        int32 n_components;
        uint16 pbo_idx_for_copy;
        bool32 is_screensize_dependent;
        // NOTE: We assume a builtin texture can belong to multiple materials,
        // but a non-builtin texture can belong to only one material.
        // If we delete a material, we delete all its non-builtin textures.
        bool32 is_builtin;
    };

    enum class MaterialState {
        empty,
        initialized,
        textures_being_copied_to_pbo,
        textures_copied_to_pbo,
        complete
    };

    struct Material {
        char name[MAX_COMMON_NAME_LENGTH];
        MaterialState state;
        bool32 have_textures_been_generated;
        bool32 is_screensize_dependent;
        shaders::Asset shader_asset;
        shaders::Asset depth_shader_asset;
        uint32 n_textures;
        Texture textures[MAX_N_TEXTURES_PER_MATERIAL];
        char texture_uniform_names[MAX_N_UNIFORMS][MAX_UNIFORM_LENGTH];
        uint32 idx_texture_uniform_names;

        v4 albedo_static;
        real32 metallic_static;
        real32 roughness_static;
        real32 ao_static;
        bool32 should_use_normal_map;
    };

    struct State {
        PersistentPbo persistent_pbo;
        TextureNamePool texture_name_pool;
        Array<Material> materials;
    };

    static char const * texture_type_to_string(TextureType texture_type);
    static TextureType texture_type_from_string(char const *str);
    static void activate_font_texture(u32 texture_name);
    static void push_font_texture(iv2 tex_coords, iv2 char_size, void const *data);
    static Texture * init_texture(
        Texture *texture,
        TextureType type,
        char const *path
    );
    static Texture * init_texture(
        Texture *texture,
        GLenum target,
        TextureType type,
        uint32 texture_name,
        int32 width,
        int32 height,
        int32 n_components
    );
    static void destroy_texture(Texture *texture);
    static TextureAtlas * init_texture_atlas(
        TextureAtlas *atlas,
        iv2 size
    );
    static iv2 push_space_to_texture_atlas(
        TextureAtlas* atlas,
        iv2 space_size
    );
    static Material * init_material(
        Material *material,
        const char *name
    );
    static void destroy_material(Material *material);
    static Material * get_material_by_name(
        Array<Material> *materials,
        char const *name
    );
    static void add_texture_to_material(
        Material *material, Texture texture, const char *uniform_name
    );
    static void bind_texture_uniforms(Material *material);
    static void delete_persistent_pbo(PersistentPbo *ppbo);
    static void init(
        mats::State *materials_state,
        MemoryPool *memory_pool
    );
    static bool32 prepare_material_and_check_if_done(
        Material *material,
        PersistentPbo *persistent_pbo,
        TextureNamePool *texture_name_pool,
        Queue<Task> *task_queue
    );
    static void reload_shaders(Array<Material> *materials);

private:
    static bool32 is_texture_type_screensize_dependent(TextureType type);
    static uint16 get_new_persistent_pbo_idx(PersistentPbo *ppbo);
    static void * get_memory_for_persistent_pbo_idx(PersistentPbo *ppbo, uint16 idx);
    static void copy_textures_to_pbo(Material *material, PersistentPbo *persistent_pbo);
    static uint32 get_new_texture_name(TextureNamePool *pool, uint32 target_size);
    static void * get_offset_for_persistent_pbo_idx(PersistentPbo *ppbo, uint16 idx);
    static void generate_textures_from_pbo(
        Material *material,
        PersistentPbo *persistent_pbo,
        TextureNamePool *texture_name_pool
    );
    static char const * material_state_to_string(MaterialState material_state);
    static PersistentPbo * init_persistent_pbo(
        PersistentPbo *ppbo,
        uint16 texture_count, int32 width, int32 height, int32 n_components
    );
    static TextureNamePool * init_texture_name_pool(
        TextureNamePool *pool,
        MemoryPool *memory_pool,
        uint32 n_textures,
        uint32 mipmap_max_level
    );
};
