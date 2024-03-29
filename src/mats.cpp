// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/pstr.h"
#include "shaders.hpp"
#include "array.hpp"
#include "util.hpp"
#include "glutil.hpp"
#include "logs.hpp"
#include "files.hpp"
#include "mats.hpp"
#include "intrinsics.hpp"


mats::State *mats::state = nullptr;


Array<mats::Material> *
mats::get_materials()
{
    return &mats::state->materials;
}


u32
mats::get_n_materials()
{
    return mats::state->materials.length;
}


mats::Material *
mats::push_material()
{
    return mats::state->materials.push();
}


void
mats::mark_start_of_non_internal_materials()
{
    mats::state->first_non_internal_material_idx = mats::state->materials.length;
}


void
mats::destroy_non_internal_materials()
{
    for (
        u32 idx = mats::state->first_non_internal_material_idx;
        idx < mats::state->materials.length;
        idx++
    ) {
        destroy_material(mats::state->materials[idx]);
    }

    mats::state->materials.delete_elements_after_index(
        mats::state->first_non_internal_material_idx);
}


bool
mats::is_material_at_idx_internal(u32 idx)
{
    return idx < mats::state->first_non_internal_material_idx;
}


char const *
mats::texture_type_to_string(TextureType texture_type) {
    if (texture_type == TextureType::none) {
        return "none";
    } else if (texture_type == TextureType::albedo) {
        return "albedo";
    } else if (texture_type == TextureType::metallic) {
        return "metallic";
    } else if (texture_type == TextureType::roughness) {
        return "roughness";
    } else if (texture_type == TextureType::ao) {
        return "ao";
    } else if (texture_type == TextureType::normal) {
        return "normal";
    } else if (texture_type == TextureType::shadowmaps_2d) {
        return "shadowmaps_2d";
    } else if (texture_type == TextureType::shadowmaps_3d) {
        return "shadowmaps_3d";
    } else if (texture_type == TextureType::other) {
        return "other";
    } else if (texture_type == TextureType::g_position) {
        return "g_position";
    } else if (texture_type == TextureType::g_normal) {
        return "g_normal";
    } else if (texture_type == TextureType::g_albedo) {
        return "g_albedo";
    } else if (texture_type == TextureType::g_pbr) {
        return "g_pbr";
    } else if (texture_type == TextureType::l_color) {
        return "l_color";
    } else if (texture_type == TextureType::l_bright_color) {
        return "l_bright_color";
    } else if (texture_type == TextureType::l_depth) {
        return "l_depth";
    } else if (texture_type == TextureType::blur1) {
        return "blur1";
    } else if (texture_type == TextureType::blur2) {
        return "blur2";
    } else {
        logs::warning("Could not convert TextureType to string: %d", texture_type);
        return "<unknown>";
    }
}


mats::TextureType
mats::texture_type_from_string(char const *str)
{
    if (strcmp(str, "none") == 0) {
        return TextureType::none;
    } else if (strcmp(str, "albedo") == 0) {
        return TextureType::albedo;
    } else if (strcmp(str, "metallic") == 0) {
        return TextureType::metallic;
    } else if (strcmp(str, "roughness") == 0) {
        return TextureType::roughness;
    } else if (strcmp(str, "ao") == 0) {
        return TextureType::ao;
    } else if (strcmp(str, "normal") == 0) {
        return TextureType::normal;
    } else if (strcmp(str, "shadowmaps_2d") == 0) {
        return TextureType::shadowmaps_2d;
    } else if (strcmp(str, "shadowmaps_3d") == 0) {
        return TextureType::shadowmaps_3d;
    } else if (strcmp(str, "other") == 0) {
        return TextureType::other;
    } else if (strcmp(str, "g_position") == 0) {
        return TextureType::g_position;
    } else if (strcmp(str, "g_normal") == 0) {
        return TextureType::g_normal;
    } else if (strcmp(str, "g_albedo") == 0) {
        return TextureType::g_albedo;
    } else if (strcmp(str, "g_pbr") == 0) {
        return TextureType::g_pbr;
    } else if (strcmp(str, "l_color") == 0) {
        return TextureType::l_color;
    } else if (strcmp(str, "l_bright_color") == 0) {
        return TextureType::l_bright_color;
    } else if (strcmp(str, "l_depth") == 0) {
        return TextureType::l_depth;
    } else if (strcmp(str, "blur1") == 0) {
        return TextureType::blur1;
    } else if (strcmp(str, "blur2") == 0) {
        return TextureType::blur2;
    } else {
        logs::warning("Could not parse TextureType from string: %s", str);
        return TextureType::none;
    }
}


void
mats::activate_font_texture(u32 texture_name)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_name);
}


void
mats::push_font_texture(iv2 tex_coords, iv2 char_size, void const *data)
{
    glTexSubImage2D(GL_TEXTURE_2D, 0, tex_coords.x, tex_coords.y,
        char_size.x, char_size.y,
        GL_RED, GL_UNSIGNED_BYTE, data);
}


mats::Texture *
mats::init_texture(
    Texture *texture,
    TextureType type,
    char const *path
) {
    texture->type = type;
    strcpy(texture->path, TEXTURE_DIR);
    strcat(texture->path, path);
    texture->target = GL_TEXTURE_2D;
    texture->is_screensize_dependent = is_texture_type_screensize_dependent(type);
    return texture;
}


mats::Texture *
mats::init_texture(
    Texture *texture,
    GLenum target,
    TextureType type,
    u32 texture_name,
    i32 width,
    i32 height,
    i32 n_components
) {
    texture->target = target;
    texture->type = type;
    texture->texture_name = texture_name;
    texture->width = width;
    texture->height = height;
    texture->n_components = n_components;
    texture->is_screensize_dependent = is_texture_type_screensize_dependent(type);
    return texture;
}


void
mats::destroy_texture(Texture *texture) {
    logs::info("Destroying texture of type %s", texture_type_to_string(texture->type));
    glDeleteTextures(1, &texture->texture_name);
}


mats::TextureAtlas *
mats::init_texture_atlas(
    TextureAtlas* atlas,
    iv2 size
) {
    atlas->size = size;
    atlas->next_position = iv2(0, 0);
    atlas->max_allocated_position_per_axis = iv2(0, 0);

    glGenTextures(1, &atlas->texture_name);
    glBindTexture(GL_TEXTURE_2D, atlas->texture_name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->size.x, atlas->size.y, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return atlas;
}

iv2
mats::push_space_to_texture_atlas(TextureAtlas *atlas, iv2 space_size)
{
    // New space in a texture is first allocated along the x-axis.
    // If we run over the end of the x-axis, we go to the next "row" from the
    // beginning of the x-axis.
    iv2 new_space_position = atlas->next_position;
    iv2 new_space_end = atlas->next_position + space_size;

    // If we run past the end of the y-axis, we've filled up the texture.
    // This is a problem. We'll start reallocating from the beginning,
    // overriding old stuff.
    if (new_space_end.y > atlas->size.y) {
        logs::error("Ran past y-axis end of TextureAtlas.");
        // Maybe we just start overwriting stuff here.
        new_space_position = iv2(0, 0);
    }

    // If we run past the end of the x-axis, move on to the next row.
    if (new_space_end.x > atlas->size.x) {
        new_space_position = iv2(0, atlas->max_allocated_position_per_axis.y);
    }

    atlas->max_allocated_position_per_axis = max(
        atlas->max_allocated_position_per_axis, new_space_end
    );
    atlas->next_position = new_space_position + iv2(space_size.x, 0);
    return new_space_position;
}


mats::Material *
mats::init_material(Material *material, const char *name)
{
    // Hardcoded values for when we can't load a texture.
    material->albedo_static = v4(-1.0f, -1.0f, -1.0f, -1.0f);
    material->metallic_static = -1.0f;
    material->roughness_static = -1.0f;
    material->ao_static = -1.0f;

    pstr_copy(material->name, MAX_COMMON_NAME_LENGTH, name);
    material->state = MaterialState::initialized;
    return material;
}


void
mats::destroy_material(Material *material)
{
    shaders::destroy_shader_asset(&material->shader_asset);

    if (shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
        shaders::destroy_shader_asset(&material->depth_shader_asset);
    }

    for (u32 idx = 0; idx < material->n_textures; idx++) {
        Texture *texture = &material->textures[idx];
        if (!texture->is_builtin) {
            destroy_texture(texture);
        }
    }

    memset(material, 0, sizeof(Material));
}


mats::Material *
mats::get_material_by_name(char const *name)
{
    each (material, mats::state->materials) {
        if (pstr_eq(material->name, name)) {
            return material;
        }
    }
    return nullptr;
}


void
mats::add_texture_to_material(Material *material, Texture texture, const char *uniform_name)
{
    if (texture.is_screensize_dependent) {
        material->is_screensize_dependent = true;
    }
    material->textures[material->n_textures++] = texture;
    pstr_copy(material->texture_uniform_names[material->idx_texture_uniform_names++],
        MAX_COMMON_NAME_LENGTH, uniform_name);
}


void
mats::bind_texture_uniforms(Material *material)
{
    shaders::Asset *shader_asset = &material->shader_asset;

    if (shader_asset->type != shaders::Type::depth) {
        glUseProgram(shader_asset->program);

        for (
            u32 uniform_idx = 0;
            uniform_idx < shader_asset->n_intrinsic_uniforms;
            uniform_idx++
        ) {
            const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
            if (strcmp(uniform_name, "should_use_normal_map") == 0) {
                shaders::set_bool(shader_asset, "should_use_normal_map", material->should_use_normal_map);
            } else if (strcmp(uniform_name, "albedo_static") == 0) {
                shaders::set_vec4(shader_asset, "albedo_static", &material->albedo_static);
            } else if (strcmp(uniform_name, "metallic_static") == 0) {
                shaders::set_float(shader_asset, "metallic_static", material->metallic_static);
            } else if (strcmp(uniform_name, "roughness_static") == 0) {
                shaders::set_float(shader_asset, "roughness_static", material->roughness_static);
            } else if (strcmp(uniform_name, "ao_static") == 0) {
                shaders::set_float(shader_asset, "ao_static", material->ao_static);
            }
        }

        shaders::reset_texture_units(shader_asset);

        for (u32 idx = 0; idx < material->n_textures; idx++) {
            Texture *texture = &material->textures[idx];
            const char *uniform_name = material->texture_uniform_names[idx];
            if (SETTINGS.shader_debug_on) {
                logs::info("Setting uniforms: (uniform_name %s) (texture->texture_name %d)",
                    uniform_name, texture->texture_name);
            }
            shaders::set_int(shader_asset, uniform_name,
                shaders::add_texture_unit(shader_asset, texture->texture_name, texture->target)
            );
        }
    }

    shader_asset->did_set_texture_uniforms = true;
}


void
mats::delete_persistent_pbo()
{
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glDeleteBuffers(1, &mats::state->persistent_pbo.pbo);
}


void
mats::init(
    mats::State *materials_state,
    memory::Pool *memory_pool
) {
    mats::state = materials_state;
    mats::state->materials = Array<Material>(memory_pool, MAX_N_MATERIALS, "materials");
    init_texture_name_pool(memory_pool, 256, 4);
    init_persistent_pbo(25, 2048, 2048, 4);
}


bool
mats::prepare_material_and_check_if_done(Material *material)
{
    if (material->state == MaterialState::empty) {
        logs::warning("Empty material '%s'. This should never happen.", material->name);
        return false;
    }

    if (material->state == MaterialState::initialized) {
        // NOTE: We only have to copy stuff if we have one or more textures that
        // don't have names yet.
        bool should_try_to_copy_textures = false;
        for (u32 idx = 0; idx < material->n_textures; idx++) {
            Texture *texture = &material->textures[idx];
            if (!texture->texture_name) {
                should_try_to_copy_textures = true;
            }
        }

        if (should_try_to_copy_textures) {
            material->state = MaterialState::textures_being_copied_to_pbo;
            tasks::push({
                .fn = (tasks::TaskFn)copy_textures_to_pbo,
                .argument_1 = (void*)material,
            });
        } else {
            material->state = MaterialState::textures_copied_to_pbo;
        }
    }

    if (material->state == MaterialState::textures_being_copied_to_pbo) {
        // Wait. The task will progress our status.
    }

    if (material->state == MaterialState::textures_copied_to_pbo) {
        generate_textures_from_pbo(material);
        material->state = MaterialState::complete;
    }

    if (material->state == MaterialState::complete) {
        // NOTE: Because the shader might be reloaded at any time, we need to
        // check whether or not we need to set any uniforms every time.
        if (!material->shader_asset.did_set_texture_uniforms) {
            bind_texture_uniforms(material);
        }

        return true;
    }

    return false;
}


void
mats::reload_shaders()
{
    memory::Pool temp_memory_pool = {};

    each (material, mats::state->materials) {
        shaders::load_shader_asset(&material->shader_asset, &temp_memory_pool);
        if (shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
            shaders::load_shader_asset(&material->depth_shader_asset, &temp_memory_pool);
        }
    }

    // NOTE: We don't reload the standard depth shader asset.
    // It makes the code simpler and we don't really need to.

    memory::destroy_memory_pool(&temp_memory_pool);
}


bool
mats::is_texture_type_screensize_dependent(TextureType type)
{
    return (
        type == TextureType::g_position ||
        type == TextureType::g_normal ||
        type == TextureType::g_albedo ||
        type == TextureType::g_pbr ||
        type == TextureType::l_color ||
        type == TextureType::l_bright_color ||
        type == TextureType::l_depth ||
        type == TextureType::blur1 ||
        type == TextureType::blur2
    );
}


u16
mats::get_new_persistent_pbo_idx()
{
    auto *ppbo = &mats::state->persistent_pbo;
    u16 current_idx = ppbo->next_idx;
    ppbo->next_idx++;
    if (ppbo->next_idx >= ppbo->texture_count) {
        ppbo->next_idx = 0;
    }
    return current_idx;
}


void *
mats::get_memory_for_persistent_pbo_idx(u16 idx)
{
    auto *ppbo = &mats::state->persistent_pbo;
    return (char*)ppbo->memory + ((u64)idx * ppbo->texture_size);
}


void
mats::copy_textures_to_pbo(Material *material)
{
    for (u32 idx = 0; idx < material->n_textures; idx++) {
        Texture *texture = &material->textures[idx];
        if (texture->texture_name) {
            continue;
        }
        unsigned char *image_data = files::load_image(texture->path, &texture->width, &texture->height,
            &texture->n_components, true);
        texture->pbo_idx_for_copy = get_new_persistent_pbo_idx();
        memcpy(get_memory_for_persistent_pbo_idx(texture->pbo_idx_for_copy),
            image_data, texture->width * texture->height * texture->n_components);
        files::free_image(image_data);
    }
    material->state = MaterialState::textures_copied_to_pbo;
}


u32
mats::get_new_texture_name(u32 target_size)
{
    auto *pool = &mats::state->texture_name_pool;
    for (u32 idx_size = 0; idx_size < pool->n_sizes; idx_size++) {
        if (pool->sizes[idx_size] == target_size) {
            assert(pool->idx_next[idx_size] < pool->n_textures);
            u32 idx_name = (idx_size * pool->n_textures) + pool->idx_next[idx_size];
            u32 texture_name = pool->texture_names[idx_name];
            pool->idx_next[idx_size]++;
            return texture_name;
        }
    }
    logs::fatal("Could not make texture of size %d as there is no pool for that size.", target_size);
    return 0;
}


void *
mats::get_offset_for_persistent_pbo_idx(u16 idx)
{
    return (void*)((u64)idx * mats::state->persistent_pbo.texture_size);
}


void
mats::generate_textures_from_pbo(Material *material)
{
    if (material->have_textures_been_generated) {
        logs::warning("Tried to generate textures but they've already been generated.");
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mats::state->persistent_pbo.pbo);

    for (u32 idx = 0; idx < material->n_textures; idx++) {
        Texture *texture = &material->textures[idx];
        if (texture->texture_name != 0) {
            continue;
        }
        texture->texture_name = get_new_texture_name(texture->width);
        glBindTexture(GL_TEXTURE_2D, texture->texture_name);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height,
            glutil::get_texture_format_from_n_components(texture->n_components),
            GL_UNSIGNED_BYTE,
            get_offset_for_persistent_pbo_idx(texture->pbo_idx_for_copy));
        glGenerateMipmap(GL_TEXTURE_2D);

        if (texture->type == TextureType::normal) {
            material->should_use_normal_map = true;
        }
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    material->have_textures_been_generated = true;
}


char const *
mats::material_state_to_string(MaterialState material_state)
{
    if (material_state == MaterialState::empty) {
        return "empty";
    } else if (material_state == MaterialState::initialized) {
        return "initialized";
    } else if (material_state == MaterialState::textures_being_copied_to_pbo) {
        return "textures being copied to pbo";
    } else if (material_state == MaterialState::textures_copied_to_pbo) {
        return "textures copied to pbo";
    } else if (material_state == MaterialState::complete) {
        return "complete";
    } else {
        return "<unknown>";
    }
}


mats::PersistentPbo *
mats::init_persistent_pbo(
    u16 texture_count, i32 width, i32 height, i32 n_components
) {
    auto *ppbo = &mats::state->persistent_pbo;
    ppbo->texture_count = texture_count;
    ppbo->width = width;
    ppbo->height = height;
    ppbo->n_components = n_components;
    ppbo->texture_size = width * height * n_components;
    ppbo->total_size = ppbo->texture_size * ppbo->texture_count;

    glGenBuffers(1, &ppbo->pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ppbo->pbo);

    GLbitfield flags;
    if (GLAD_GL_ARB_buffer_storage) {
        flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_UNPACK_BUFFER, ppbo->total_size, 0, flags);
    } else {
        flags = GL_MAP_WRITE_BIT;
        glBufferData(GL_PIXEL_UNPACK_BUFFER, ppbo->total_size, 0, GL_DYNAMIC_DRAW);
    }

    ppbo->memory = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, ppbo->total_size, flags);

    if (ppbo->memory == nullptr) {
        logs::fatal("Could not get memory for PBO.");
    }

    // We need to unbind this or it will mess up some textures transfers
    // after this function.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    return ppbo;
}


mats::TextureNamePool *
mats::init_texture_name_pool(
    memory::Pool *memory_pool,
    u32 n_textures,
    u32 mipmap_max_level
) {
    auto *pool = &mats::state->texture_name_pool;
    pool->n_textures = n_textures;
    pool->mipmap_max_level = mipmap_max_level;
    pool->n_sizes = 0;
    pool->sizes[pool->n_sizes++] = 256;
    pool->sizes[pool->n_sizes++] = 512;
    pool->sizes[pool->n_sizes++] = 2048;

    pool->texture_names = (u32*)memory::push(memory_pool,
        sizeof(u32) * pool->n_textures * pool->n_sizes, "texture_names");

    glGenTextures(pool->n_textures * pool->n_sizes, pool->texture_names);

    range_named (idx_size, 0, pool->n_sizes) {
        range_named (idx_texture, 0, pool->n_textures) {
            u32 idx_name = (idx_size * pool->n_textures) + idx_texture;
            glBindTexture(GL_TEXTURE_2D, pool->texture_names[idx_name]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pool->mipmap_max_level);
            glTexStorage2D(GL_TEXTURE_2D, pool->mipmap_max_level + 1, GL_RGBA8,
                pool->sizes[idx_size], pool->sizes[idx_size]);
        }
    }

    return pool;
}
