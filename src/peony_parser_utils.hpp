// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "array.hpp"
#include "peony_parser.hpp"
#include "models.hpp"
#include "renderer.hpp"

class peony_parser_utils {
public:
    static constexpr const char *TEXTURE_PREFIX = "textures.";
    static constexpr size_t TEXTURE_PREFIX_LENGTH = 9;
    static constexpr const char *BUILTIN_TEXTURE_PREFIX = "builtin_textures.";
    static constexpr size_t BUILTIN_TEXTURE_PREFIX_LENGTH = 17;

    static char * get_string(peony_parser::Prop *prop);
    static bool32 * get_boolean(peony_parser::Prop *prop);
    static real32 * get_number(peony_parser::Prop *prop);
    static v2 * get_vec2(peony_parser::Prop *prop);
    static v3 * get_vec3(peony_parser::Prop *prop);
    static v4 * get_vec4(peony_parser::Prop *prop);
    static peony_parser::Prop * find_prop(peony_parser::Entry *entry, char const *name);
    static void get_unique_string_values_for_prop_name(
        peony_parser::PeonyFile *pf,
        Array<char[MAX_COMMON_NAME_LENGTH]> *unique_values,
        char const *prop_name
    );
    static void create_material_from_peony_file_entry(
        mats::Material *material,
        peony_parser::Entry *entry,
        memory::Pool *memory_pool
    );
    static void create_model_loader_from_peony_file_entry(
        peony_parser::Entry *entry,
        entities::Handle entity_handle,
        models::ModelLoader *model_loader
    );
    static void create_entity_loader_from_peony_file_entry(
        peony_parser::Entry *entry,
        entities::Handle entity_handle,
        models::EntityLoader *entity_loader
    );
};
