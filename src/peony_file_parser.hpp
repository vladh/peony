#ifndef PEONY_FILE_PARSER_HPP
#define PEONY_FILE_PARSER_HPP

namespace PeonyFileParser {
  constexpr uint32 MAX_TOKEN_LENGTH = 128;
  constexpr uint32 MAX_N_ARRAY_VALUES = 16;
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

  constexpr const char *TEXTURE_PREFIX = "textures.";
  constexpr size_t TEXTURE_PREFIX_LENGTH = 9;
  constexpr const char *MATERIAL_FILE_DIRECTORY = "data/materials/";
  constexpr const char *MATERIAL_FILE_EXTENSION = ".peony_materials";

  class MaterialEntries {
  public:
    char shader_asset_vert_path[MAX_TOKEN_LENGTH];
    char shader_asset_frag_path[MAX_TOKEN_LENGTH];
    char shader_asset_geom_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_vert_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_frag_path[MAX_TOKEN_LENGTH];
    char depth_shader_asset_geom_path[MAX_TOKEN_LENGTH];

    glm::vec4 albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
    real32 metallic_static = -1.0f;
    real32 roughness_static = -1.0f;
    real32 ao_static = -1.0f;

    uint32 n_textures = 0;
    char texture_uniform_names[MAX_N_ARRAY_VALUES][MAX_TOKEN_LENGTH];
    TextureType texture_types[MAX_N_ARRAY_VALUES];
    char texture_paths[MAX_N_ARRAY_VALUES][MAX_TOKEN_LENGTH];

    void print();
  };

  class SceneEntityEntries {
  public:
    char entity_debug_name[MAX_TOKEN_LENGTH];
    char model_path[MAX_TOKEN_LENGTH];
    uint32 n_materials = 0;
    MaterialEntries material_entries[MAX_N_ARRAY_VALUES];
    uint32 n_render_passes = 0;
    RenderPass::Flag render_passes[MAX_N_ARRAY_VALUES];
    // NOTE: The `entity_handle` and `parent_entity_handle` properties
    // must be filled in later!
    SpatialComponent spatial_component;

    void print();
  };

  enum class PropValueType {unknown, string, boolean, number, vec2, vec3, vec4};

  union PropValue {
    char string_value[MAX_TOKEN_LENGTH];
    bool32 boolean_value;
    real32 number_value;
    glm::vec2 vec2_value;
    glm::vec3 vec3_value;
    glm::vec4 vec4_value;
  };

  void print_value(PropValue value, PropValueType type);
  bool32 is_char_whitespace(const char target);
  bool32 is_token_whitespace(const char *token);
  bool32 is_char_allowed_in_name(const char target);
  bool32 is_token_name(const char *token);
  bool32 is_char_token_boundary(char target);
  void get_value_from_token(
    char *token,
    FILE *f,
    PropValueType *prop_value_type,
    PropValue *prop_value
  );
  bool32 get_token(char *token, FILE *f);
  bool32 get_non_trivial_token(char *token, FILE *f);
  void parse_header(char *token, FILE *f);
  void parse_vec2(char *token, FILE *f, glm::vec2 *parsed_vector);
  void parse_vec3(char *token, FILE *f, glm::vec3 *parsed_vector);
  void parse_vec4(char *token, FILE *f, glm::vec4 *parsed_vector);
  uint32 parse_property(
    char *token,
    FILE *f,
    char prop_name[MAX_TOKEN_LENGTH],
    PropValueType prop_value_types[MAX_N_ARRAY_VALUES],
    PropValue prop_values[MAX_N_ARRAY_VALUES]
  );
  void parse_material_file(
    const char *path, MaterialEntries *material_entries
  );
  uint32 parse_scene_file(
    const char *path, SceneEntityEntries *scene_entity_entries
  );
};

#endif