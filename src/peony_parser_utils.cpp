// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/pstr.h"
#include "peony_parser_utils.hpp"
#include "logs.hpp"
#include "constants.hpp"
#include "renderer.hpp"
#include "intrinsics.hpp"


char *
peony_parser_utils::get_string(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return prop->values[0].string_value;
}


bool *
peony_parser_utils::get_boolean(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return &prop->values[0].boolean_value;
}


f32 *
peony_parser_utils::get_number(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return &prop->values[0].number_value;
}


v2 *
peony_parser_utils::get_vec2(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return &prop->values[0].vec2_value;
}


v3 *
peony_parser_utils::get_vec3(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return &prop->values[0].vec3_value;
}


v4 *
peony_parser_utils::get_vec4(peony_parser::Prop *prop)
{
    if (!prop) { return nullptr; }
    return &prop->values[0].vec4_value;
}


peony_parser::Prop *
peony_parser_utils::find_prop(peony_parser::Entry *entry, char const *name)
{
    range (0, entry->n_props) {
        if (pstr_eq(name, entry->props[idx].name)) {
            return &entry->props[idx];
        }
    }
    logs::warning("Could not find prop %s", name);
    return nullptr;
}


void
peony_parser_utils::get_unique_string_values_for_prop_name(
    peony_parser::PeonyFile *pf,
    Array<char[MAX_COMMON_NAME_LENGTH]> *unique_values,
    char const *prop_name
) {
    range_named (idx_entry, 0, pf->n_entries) {
        peony_parser::Entry *entry = &pf->entries[idx_entry];
        peony_parser::Prop *prop = find_prop(entry, prop_name);
        if (!prop) {
            continue;
        }
        range_named (idx_value, 0, prop->n_values) {
            peony_parser::PropValue *value = &prop->values[idx_value];
            bool does_material_already_exist = false;
            each (unique_value, *unique_values) {
                if (pstr_eq(value->string_value, *unique_value)) {
                    does_material_already_exist = true;
                    break;
                }
            }
            if (!does_material_already_exist) {
                pstr_copy(*(unique_values->push()),
                    MAX_COMMON_NAME_LENGTH, value->string_value);
            }
        }
    }
}


void
peony_parser_utils::create_material_from_peony_file_entry(
    mats::Material *material,
    peony_parser::Entry *entry,
    memory::Pool *memory_pool
) {
    mats::init_material(material, entry->name);

    // We're calling `find_prop()` a lot here, which goes through the full
    // list of props every time, and so this is kind of #slow. Not a huge deal, but
    // good to keep in mind.
    peony_parser::Prop *prop;

    if ((prop = find_prop(entry, "albedo_static"))) {
        material->albedo_static = *get_vec4(prop);
    }
    if ((prop = find_prop(entry, "metallic_static"))) {
        material->metallic_static = *get_number(prop);
    }
    if ((prop = find_prop(entry, "roughness_static"))) {
        material->roughness_static = *get_number(prop);
    }
    if ((prop = find_prop(entry, "ao_static"))) {
        material->ao_static = *get_number(prop);
    }

    if ((prop = find_prop(entry, "shader_asset.vert_path"))) {
        if (!pstr_is_empty(get_string(prop))) {
            shaders::init_shader_asset(&material->shader_asset,
                memory_pool,
                entry->name,
                shaders::Type::standard,
                get_string(find_prop(entry, "shader_asset.vert_path")),
                get_string(find_prop(entry, "shader_asset.frag_path")),
                get_string(find_prop(entry, "shader_asset.geom_path")));
        }
    }

    if ((prop = find_prop(entry, "depth_shader_asset.vert_path"))) {
        if (!pstr_is_empty(get_string(prop))) {
            shaders::init_shader_asset(&material->depth_shader_asset,
                memory_pool,
                entry->name,
                shaders::Type::depth,
                get_string(find_prop(entry, "depth_shader_asset.vert_path")),
                get_string(find_prop(entry, "depth_shader_asset.frag_path")),
                get_string(find_prop(entry, "depth_shader_asset.geom_path")));
        }
    }

    auto *builtin_textures = renderer::get_builtin_textures();

    // Iterate through all props to get textures, since those could have any name
    range (0, entry->n_props) {
        prop = &entry->props[idx];

        if (pstr_starts_with(prop->name, TEXTURE_PREFIX)) {
            // Handle a texture
            mats::Texture texture = {};
            // The uniform name is the prop name without the prefix
            // The first value is the type, and the second the path
            // e.g. textures.foam_texture = [other, water_foam.png]
            char const *uniform_name = &prop->name[TEXTURE_PREFIX_LENGTH];
            mats::init_texture(&texture,
                mats::texture_type_from_string(prop->values[0].string_value),
                prop->values[1].string_value);
            mats::add_texture_to_material(material, texture, uniform_name);
        } else if (pstr_starts_with(prop->name, BUILTIN_TEXTURE_PREFIX)) {
            // Handle a builtin texture
            char const *builtin_uniform_name = &prop->name[BUILTIN_TEXTURE_PREFIX_LENGTH];
            if (pstr_eq(builtin_uniform_name, "g_position_texture")) {
                mats::add_texture_to_material(
                    material, *builtin_textures->g_position_texture, builtin_uniform_name);
            } else if (pstr_eq(builtin_uniform_name, "g_albedo_texture")) {
                mats::add_texture_to_material(
                    material, *builtin_textures->g_albedo_texture, builtin_uniform_name);
            } else if (pstr_eq(builtin_uniform_name, "shadowmaps_3d")) {
                mats::add_texture_to_material(
                    material, *builtin_textures->shadowmaps_3d_texture, builtin_uniform_name);
            } else if (pstr_eq(builtin_uniform_name, "shadowmaps_2d")) {
                mats::add_texture_to_material(
                    material, *builtin_textures->shadowmaps_2d_texture, builtin_uniform_name);
            } else {
                logs::fatal("Attempted to use unsupported built-in texture %s",
                    builtin_uniform_name);
            }
        }
    }
}


void
peony_parser_utils::create_model_loader_from_peony_file_entry(
    peony_parser::Entry *entry,
    entities::Handle entity_handle,
    models::ModelLoader *model_loader
) {
    peony_parser::Prop *model_path_prop = find_prop(entry, "model_path");
    assert(model_path_prop);
    char const *model_path = get_string(model_path_prop);

    models::init_model_loader(model_loader, model_path);

    peony_parser::Prop *materials_prop = find_prop(entry, "materials");
    model_loader->n_material_names = materials_prop->n_values;
    range (0, materials_prop->n_values) {
        pstr_copy(model_loader->material_names[idx],
            MAX_COMMON_NAME_LENGTH,
            materials_prop->values[idx].string_value);
    }
}


void
peony_parser_utils::create_entity_loader_from_peony_file_entry(
    peony_parser::Entry *entry,
    entities::Handle entity_handle,
    models::EntityLoader *entity_loader
) {
    peony_parser::Prop *model_path_prop = find_prop(entry, "model_path");
    assert(model_path_prop);
    char const *model_path = get_string(model_path_prop);

    // Get render pass
    auto render_pass = drawable::Pass::none;
    peony_parser::Prop *render_passes_prop = find_prop(entry, "render_passes");
    if (render_passes_prop) {
        range (0, render_passes_prop->n_values) {
            render_pass = (drawable::Pass)(
                (u32)render_pass |
                (u32)drawable::render_pass_from_string(
                    render_passes_prop->values[idx].string_value));
        }
    } else {
        logs::warning(
            "Loading EntityLoader with no RenderPasses, you probably don't want this?");
    }

    // Initialise everything except the components
    models::init_entity_loader(entity_loader, entry->name, model_path,
        render_pass, entity_handle);

    // Build physics::Component, spatial::Component, lights::Component, behavior::Component
    range (0, entry->n_props) {
        peony_parser::Prop *prop = &entry->props[idx];
        if (pstr_eq(prop->name, "physics_component.obb.center")) {
            entity_loader->physics_component.obb.center = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "physics_component.obb.x_axis")) {
            entity_loader->physics_component.obb.x_axis = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "physics_component.obb.y_axis")) {
            entity_loader->physics_component.obb.y_axis = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "physics_component.obb.extents")) {
            entity_loader->physics_component.obb.extents = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "spatial_component.position")) {
            entity_loader->spatial_component.position = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "spatial_component.rotation")) {
            entity_loader->spatial_component.rotation = glm::angleAxis(
                    radians((*get_vec4(prop))[0]),
                    v3((*get_vec4(prop))[1],
                        (*get_vec4(prop))[2],
                        (*get_vec4(prop))[3]));
        } else if (pstr_eq(prop->name, "spatial_component.scale")) {
            entity_loader->spatial_component.scale = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "light_component.type")) {
            entity_loader->light_component.type =
                lights::light_type_from_string(get_string(prop));
        } else if (pstr_eq(prop->name, "light_component.direction")) {
            entity_loader->light_component.direction = *get_vec3(prop);
        } else if (pstr_eq(prop->name, "light_component.color")) {
            entity_loader->light_component.color = *get_vec4(prop);
        } else if (pstr_eq(prop->name, "light_component.attenuation")) {
            entity_loader->light_component.attenuation = *get_vec4(prop);
        } else if (pstr_eq(prop->name, "behavior_component.behavior")) {
            entity_loader->behavior_component.behavior =
                behavior::behavior_from_string(get_string(prop));
        }
    }
}
