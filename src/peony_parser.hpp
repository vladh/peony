#pragma once

#include "types.hpp"
#include "constants.hpp"
#include "models.hpp"
#include "materials.hpp"

namespace peony_parser {
  constexpr uint32 MAX_N_FILE_ENTRIES = 128;

  constexpr const char TOKEN_SPACE = ' ';
  constexpr const char TOKEN_NEWLINE = '\n';
  constexpr const char TOKEN_EQUALS = '=';
  constexpr const char TOKEN_HEADER_START = '>';
  constexpr const char TOKEN_ARRAY_START = '[';
  constexpr const char TOKEN_ARRAY_END = ']';
  constexpr const char TOKEN_OBJECT_START = '{';
  constexpr const char TOKEN_OBJECT_END = '}';
  constexpr const char TOKEN_TUPLE_START = '(';
  constexpr const char TOKEN_TUPLE_END = ')';
  constexpr const char TOKEN_ELEMENT_SEPARATOR = ',';
  constexpr const char TOKEN_COMMENT_START = ';';

  constexpr const char *TEXTURE_PREFIX = "textures.";
  constexpr size_t TEXTURE_PREFIX_LENGTH = 9;
  constexpr const char *BUILTIN_TEXTURE_PREFIX = "builtin_textures.";
  constexpr size_t BUILTIN_TEXTURE_PREFIX_LENGTH = 17;

  struct MaterialTemplate {
    char name[MAX_TOKEN_LENGTH];
    char shader_asset_vert_path[MAX_TOKEN_LENGTH];
    char shader_asset_frag_path[MAX_TOKEN_LENGTH];
    char shader_asset_geom_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_vert_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_frag_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_geom_path[MAX_TOKEN_LENGTH];

    v4 albedo_static = v4(-1.0f, -1.0f, -1.0f, -1.0f);
    real32 metallic_static = -1.0f;
    real32 roughness_static = -1.0f;
    real32 ao_static = -1.0f;

    uint32 n_textures;
    char texture_uniform_names[MAX_N_PEONY_ARRAY_VALUES][MAX_TOKEN_LENGTH];
    TextureType texture_types[MAX_N_PEONY_ARRAY_VALUES];
    char texture_paths[MAX_N_PEONY_ARRAY_VALUES][MAX_TOKEN_LENGTH];

    uint32 n_builtin_textures;
    char builtin_texture_names[MAX_N_PEONY_ARRAY_VALUES][MAX_TOKEN_LENGTH];
  };

  struct EntityTemplate {
    char entity_debug_name[MAX_TOKEN_LENGTH];
    char model_path_or_builtin_model_name[MAX_TOKEN_LENGTH];
    ModelSource model_source;
    StackArray<char[MAX_TOKEN_LENGTH], MAX_N_PEONY_ARRAY_VALUES> material_names;
    RenderPass render_pass;
    // NOTE: The `entity_handle` and `parent_entity_handle` properties
    // must be filled in later!
    SpatialComponent spatial_component;
    LightComponent light_component;
    BehaviorComponent behavior_component;
    PhysicsComponent physics_component;
  };

  enum class PropValueType {unknown, string, boolean, number, vec2, vec3, vec4};

  union PropValue {
    char string_value[MAX_TOKEN_LENGTH];
    bool32 boolean_value;
    real32 number_value;
    v2 vec2_value;
    v3 vec3_value;
    v4 vec4_value;
  };

  void get_material_path(char *path, const char *name);
  void print_material_template(MaterialTemplate *material_template);
  void print_entity_template(EntityTemplate *entity_template);
  void parse_material_file(
    const char *path, MaterialTemplate *material_template
  );
  bool32 parse_scene_file(
    const char *path,
    StackArray<EntityTemplate, 128> *entity_templates,
    uint32 *n_entities
  );
  void create_material_from_template(
    Material *material,
    MaterialTemplate *material_template,
    BuiltinTextures *builtin_textures,
    MemoryPool *memory_pool
  );
  void create_model_loader_from_entity_template(
    EntityTemplate *entity_template,
    EntityHandle entity_handle,
    Array<ModelLoader> *model_loaders
  );
  void create_entity_loader_from_entity_template(
    EntityTemplate *entity_template,
    EntityHandle entity_handle,
    EntityLoaderSet *entity_loader_set
  );
}

using peony_parser::MaterialTemplate, peony_parser::EntityTemplate,
  peony_parser::PropValueType, peony_parser::PropValue;
