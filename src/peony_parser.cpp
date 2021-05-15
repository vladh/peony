namespace peony_parser {
  // -----------------------------------------------------------
  // Constants
  // -----------------------------------------------------------
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


  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
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
    RenderPassFlag render_pass;
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


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void print_value(PropValue value, PropValueType type) {
    if (type == PropValueType::unknown) {
      logs::info("<unknown>");
    } else if (type == PropValueType::string) {
      logs::info("%s", value.string_value);
    } else if (type == PropValueType::boolean) {
      logs::info("%d", value.boolean_value);
    } else if (type == PropValueType::number) {
      logs::info("%f", value.number_value);
    } else if (type == PropValueType::vec2) {
      logs::print_v2(&value.vec2_value);
    } else if (type == PropValueType::vec3) {
      logs::print_v3(&value.vec3_value);
    } else if (type == PropValueType::vec4) {
      logs::print_v4(&value.vec4_value);
    } else {
      logs::info("<invalid>");
    }
  }


  bool32 is_char_whitespace(const char target) {
    return target == TOKEN_NEWLINE ||
      target == TOKEN_SPACE;
  }


  bool32 is_token_whitespace(const char *token) {
    return is_char_whitespace(token[0]);
  }


  bool32 is_char_allowed_in_name(const char target) {
    return isalpha(target) ||
      isdigit(target) ||
      target == '_' ||
      target == '-' ||
      target == '/' ||
      target == '.';
  }


  bool32 is_token_name(const char *token) {
    for_range (0, strlen(token)) {
      if (!is_char_allowed_in_name(token[idx])) {
        return false;
      }
    }
    return true;
  }


  bool32 is_char_token_boundary(char target) {
    return is_char_whitespace(target) ||
      target == TOKEN_HEADER_START ||
      target == TOKEN_ELEMENT_SEPARATOR ||
      target == TOKEN_TUPLE_START ||
      target == TOKEN_TUPLE_END ||
      target == TOKEN_ARRAY_START ||
      target == TOKEN_ARRAY_END ||
      target == TOKEN_OBJECT_START ||
      target == TOKEN_OBJECT_END;
  }


  bool32 get_token(char *token, FILE *f) {
    uint32 idx_token = 0;
    bool32 could_get_token = true;

    while (true) {
      char new_char = (char)fgetc(f);

      *(token + idx_token) = new_char;
      idx_token++;

      if (is_char_token_boundary(new_char)) {
        if (idx_token - 1 == 0) {
          // Our first character is already a boundary. Radical!
        } else {
          // Return the boundary to the buffer so we can put it in its own token.
          ungetc(new_char, f);
          *(token + idx_token - 1) = '\0';
        }
        break;
      }

      if (new_char == EOF) {
        could_get_token = false;
        break;
      }

      if (idx_token >= MAX_TOKEN_LENGTH) {
        logs::error("Reached max token length for token %s.", token);
        break;
      }
    }

    *(token + idx_token) = '\0';

    return could_get_token;
  }


  bool32 get_non_trivial_token(char *token, FILE *f) {
    bool32 could_get_token;
    do {
      could_get_token = get_token(token, f);
      if (token[0] == TOKEN_COMMENT_START) {
        while ((could_get_token = get_token(token, f)) != false) {
          if (token[0] == TOKEN_NEWLINE) {
            break;
          }
        }
      }
    } while (is_token_whitespace(token) || token[0] == TOKEN_ELEMENT_SEPARATOR);
    return could_get_token;
  }


  void parse_vec2(char *token, FILE *f, v2 *parsed_vector) {
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_START);
    get_non_trivial_token(token, f);
    (*parsed_vector).x = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).y = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_END);
  }


  void parse_vec3(char *token, FILE *f, v3 *parsed_vector) {
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_START);
    get_non_trivial_token(token, f);
    (*parsed_vector).x = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).y = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).z = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_END);
  }


  void parse_vec4(char *token, FILE *f, v4 *parsed_vector) {
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_START);
    get_non_trivial_token(token, f);
    (*parsed_vector).x = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).y = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).z = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).w = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_END);
  }


  void get_value_from_token(
    char *token,
    FILE *f,
    PropValueType *prop_value_type,
    PropValue *prop_value
  ) {
    // NOTE: Type names the can start a value: vec2, vec3, vec4
    if (str::eq(token, "vec2")) {
      *prop_value_type = PropValueType::vec2;
      parse_vec2(token, f, &prop_value->vec2_value);
    } else if (str::eq(token, "vec3")) {
      *prop_value_type = PropValueType::vec3;
      parse_vec3(token, f, &prop_value->vec3_value);
    } else if (str::eq(token, "vec4")) {
      *prop_value_type = PropValueType::vec4;
      parse_vec4(token, f, &prop_value->vec4_value);
    } else if (str::eq(token, "true")) {
      *prop_value_type = PropValueType::boolean;
      prop_value->boolean_value = true;
    } else if (str::eq(token, "false")) {
      *prop_value_type = PropValueType::boolean;
      prop_value->boolean_value = false;
    } else if (str::eq(token, "0.0") || strtod(token, nullptr) != 0.0f) {
      // NOTE: `strtod()` returns 0.0 if parsing fails, so we need to check
      // if our value actually was 0.0;
      *prop_value_type = PropValueType::number;
      prop_value->number_value = (real32)strtod(token, nullptr);
    } else {
      *prop_value_type = PropValueType::string;
      strcpy(prop_value->string_value, token);
    }
  }


  void parse_header(char *token, FILE *f) {
    get_non_trivial_token(token, f);
    assert(is_token_name(token));
  }


  uint32 parse_property(
    char *token,
    FILE *f,
    char prop_name[MAX_TOKEN_LENGTH],
    PropValueType prop_value_types[MAX_N_PEONY_ARRAY_VALUES],
    PropValue prop_values[MAX_N_PEONY_ARRAY_VALUES]
  ) {
    uint32 n_values = 0;
    strcpy(prop_name, token);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_EQUALS);
    get_non_trivial_token(token, f);

    assert(
      is_token_name(token) || token[0] == TOKEN_ARRAY_START
    );

    if (is_token_name(token)) {
      get_value_from_token(
        token,
        f,
        &prop_value_types[n_values],
        &prop_values[n_values]
      );
      n_values++;
    } else if (token[0] == TOKEN_ARRAY_START) {
      while (true) {
        get_non_trivial_token(token, f);
        if (token[0] == TOKEN_ARRAY_END) {
          break;
        }
        get_value_from_token(
          token,
          f,
          &prop_value_types[n_values],
          &prop_values[n_values]
        );
        n_values++;
      }
    }

    return n_values;
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  void get_material_path(char *path, const char *name) {
    strcpy(path, MATERIAL_FILE_DIRECTORY);
    strcat(path, name);
    strcat(path, MATERIAL_FILE_EXTENSION);
  }

  void print_material_template(MaterialTemplate *material_template) {
    logs::info("MaterialTemplate");
    logs::info("  name: %s", material_template->name);
    logs::info("  shader_asset_vert_path: %s", material_template->shader_asset_vert_path);
    logs::info("  shader_asset_frag_path: %s", material_template->shader_asset_frag_path);
    logs::info("  shader_asset_geom_path: %s", material_template->shader_asset_geom_path);
    logs::info(
      "  depth_shader_asset_vert_path: %s",
      material_template->depth_shader_asset_vert_path
    );
    logs::info(
      "  depth_shader_asset_frag_path: %s",
      material_template->depth_shader_asset_frag_path
    );
    logs::info(
      "  depth_shader_asset_geom_path: %s",
      material_template->depth_shader_asset_geom_path
    );
    logs::info("  albedo_static:");
    logs::print_v4(&material_template->albedo_static);
    logs::info("  metallic_static: %f", material_template->metallic_static);
    logs::info("  roughness_static: %f", material_template->roughness_static);
    logs::info("  ao_static: %f", material_template->ao_static);
    logs::info("  n_textures: %d", material_template->n_textures);
    for (
      uint32 idx_texture = 0;
      idx_texture < material_template->n_textures;
      idx_texture++
    ) {
      logs::info(
        "  texture %s (%s, %s)",
        material_template->texture_uniform_names[idx_texture],
        materials::texture_type_to_string(
          material_template->texture_types[idx_texture]
        ),
        material_template->texture_paths[idx_texture]
      );
    }
    logs::info("  n_builtin_textures: %d", material_template->n_builtin_textures);
    for (
      uint32 idx_texture = 0;
      idx_texture < material_template->n_builtin_textures;
      idx_texture++
    ) {
      logs::info(
        "  built-in texture %s",
        material_template->builtin_texture_names[idx_texture]
      );
    }
  }


  void print_entity_template(EntityTemplate *entity_template) {
    logs::info("EntityTemplate");
    logs::info("  name: %s", entity_template->entity_debug_name);
    logs::info(
      "  model_path_or_builtin_model_name: %s",
      entity_template->model_path_or_builtin_model_name
    );
    logs::info("  model_source: %d", entity_template->model_source);
    logs::info("  material_names.length: %d", entity_template->material_names.length);
    logs::info("  material_names:");
    for_each (material_name, entity_template->material_names) {
      logs::info(*material_name);
    }
    logs::info("  render_pass: %d", entity_template->render_pass);
    spatial::print_spatial_component(&entity_template->spatial_component);
  }


  void parse_material_file(
    const char *path, MaterialTemplate *material_template
  ) {
    FILE *f = fopen(path, "r");

    if (!f) {
      logs::fatal("Could not open file %s.", path);
      return;
    }

    char token[MAX_TOKEN_LENGTH];
    uint32 n_values;
    char prop_name[MAX_TOKEN_LENGTH];
    PropValueType prop_value_types[MAX_N_PEONY_ARRAY_VALUES];
    PropValue prop_values[MAX_N_PEONY_ARRAY_VALUES];

    while (get_non_trivial_token(token, f)) {
      if (token[0] == TOKEN_HEADER_START) {
        parse_header(token, f);
        strcpy(material_template->name, token);
      } else if (is_token_name(token)) {
        n_values = parse_property(
          token, f, prop_name, prop_value_types, prop_values
        );

        if (str::eq(prop_name, "shader_asset.vert_path")) {
          strcpy(
            material_template->shader_asset_vert_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "shader_asset.frag_path")) {
          strcpy(
            material_template->shader_asset_frag_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "shader_asset.geom_path")) {
          strcpy(
            material_template->shader_asset_geom_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "depth_shader_asset.vert_path")) {
          strcpy(
            material_template->depth_shader_asset_vert_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "depth_shader_asset.frag_path")) {
          strcpy(
            material_template->depth_shader_asset_frag_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "depth_shader_asset.geom_path")) {
          strcpy(
            material_template->depth_shader_asset_geom_path,
            prop_values[0].string_value
          );
        } else if (str::eq(prop_name, "albedo_static")) {
          material_template->albedo_static = prop_values[0].vec4_value;
        } else if (str::eq(prop_name, "metallic_static")) {
          material_template->metallic_static =
            prop_values[0].number_value;
        } else if (str::eq(prop_name, "roughness_static")) {
          material_template->roughness_static =
            prop_values[0].number_value;
        } else if (str::eq(prop_name, "ao_static")) {
          material_template->ao_static =
            prop_values[0].number_value;
        } else if (
          strncmp(prop_name, TEXTURE_PREFIX, TEXTURE_PREFIX_LENGTH) == 0
        ) {
          uint32 idx_texture = material_template->n_textures;
          strcpy(
            material_template->texture_uniform_names[idx_texture],
            prop_name + TEXTURE_PREFIX_LENGTH
          );
          material_template->texture_types[idx_texture] =
            materials::texture_type_from_string(prop_values[0].string_value);
          strcpy(
            material_template->texture_paths[idx_texture],
            prop_values[1].string_value
          );
          material_template->n_textures++;
        } else if (
          strncmp(
            prop_name, BUILTIN_TEXTURE_PREFIX, BUILTIN_TEXTURE_PREFIX_LENGTH
          ) == 0
        ) {
          uint32 idx_texture = material_template->n_builtin_textures;
          strcpy(
            material_template->builtin_texture_names[idx_texture],
            prop_name + BUILTIN_TEXTURE_PREFIX_LENGTH
          );
          material_template->n_builtin_textures++;
        } else {
          logs::info("Unhandled prop_name %s with values:", prop_name);
          for_range_named (idx_value, 0, n_values) {
            print_value(prop_values[idx_value], prop_value_types[idx_value]);
          }
        }
      } else {
        logs::info("Unhandled token: %s", token);
      }
    }

    fclose(f);
  }


  bool32 parse_scene_file(
    const char *path,
    StackArray<EntityTemplate, 128> *entity_templates,
    uint32 *n_entities
  ) {
    *n_entities = 0;
    int32 idx_entity = -1;

    FILE *f = fopen(path, "r");

    if (!f) {
      logs::error("Could not open file %s.", path);
      return false;
    }

    EntityTemplate *entity_template = nullptr;
    char token[MAX_TOKEN_LENGTH];
    uint32 n_values;
    char prop_name[MAX_TOKEN_LENGTH];
    PropValueType prop_value_types[MAX_N_PEONY_ARRAY_VALUES];
    PropValue prop_values[MAX_N_PEONY_ARRAY_VALUES];

    while (get_non_trivial_token(token, f)) {
      if (token[0] == TOKEN_HEADER_START) {
        idx_entity++;
        entity_template = entity_templates->push();
        *entity_template = {};
        parse_header(token, f);
        strcpy(
          entity_template->entity_debug_name,
          token
        );
      } else if (is_token_name(token)) {
        if (entity_template == nullptr) {
          logs::fatal("Tried to parse file, but encountered data before header.");
          assert(false); // A little hint for the compiler
        }

        n_values = parse_property(
          token, f, prop_name, prop_value_types, prop_values
        );

        if (str::eq(prop_name, "model_path")) {
          strcpy(
            entity_template->model_path_or_builtin_model_name,
            prop_values[0].string_value
          );
          entity_template->model_source = ModelSource::file;
        } else if (str::eq(prop_name, "builtin_model_name")) {
          strcpy(
            entity_template->model_path_or_builtin_model_name,
            prop_values[0].string_value
          );
          entity_template->model_source = ModelSource::data;
        } else if (str::eq(prop_name, "materials")) {
          for_range_named (idx_value, 0, n_values) {
            strcpy(
              *(entity_template->material_names.push()),
              prop_values[idx_value].string_value
            );
          }
        } else if (str::eq(prop_name, "render_passes")) {
          RenderPassFlag render_pass = RenderPass::none;
          for_range_named (idx_value, 0, n_values) {
            render_pass = render_pass |
              models::render_pass_from_string(prop_values[idx_value].string_value);
          }
          entity_template->render_pass = render_pass;
        } else if (str::eq(prop_name, "physics_component.obb.center")) {
          entity_template->physics_component.obb.center = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "physics_component.obb.x_axis")) {
          entity_template->physics_component.obb.x_axis = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "physics_component.obb.y_axis")) {
          entity_template->physics_component.obb.y_axis = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "physics_component.obb.extents")) {
          entity_template->physics_component.obb.extents = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "spatial_component.position")) {
          entity_template->spatial_component.position = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "spatial_component.rotation")) {
          entity_template->spatial_component.rotation =
            glm::angleAxis(
              radians(prop_values[0].vec4_value[0]),
              v3(
                prop_values[0].vec4_value[1],
                prop_values[0].vec4_value[2],
                prop_values[0].vec4_value[3]
              )
            );
        } else if (str::eq(prop_name, "spatial_component.scale")) {
          entity_template->spatial_component.scale = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "light_component.type")) {
          entity_template->light_component.type =
            lights::light_type_from_string(prop_values[0].string_value);
        } else if (str::eq(prop_name, "light_component.direction")) {
          entity_template->light_component.direction = prop_values[0].vec3_value;
        } else if (str::eq(prop_name, "light_component.color")) {
          entity_template->light_component.color = prop_values[0].vec4_value;
        } else if (str::eq(prop_name, "light_component.attenuation")) {
          entity_template->light_component.attenuation = prop_values[0].vec4_value;
        } else if (str::eq(prop_name, "behavior_component.behavior")) {
          entity_template->behavior_component.behavior =
            behavior::behavior_from_string(prop_values[0].string_value);
        } else {
          logs::info("Unhandled prop_name %s with values:", prop_name);
          for_range_named (idx_value, 0, n_values) {
            print_value(prop_values[idx_value], prop_value_types[idx_value]);
          }
        }
      } else {
        logs::info("Unhandled token: %s", token);
      }
    }

    fclose(f);

    *n_entities = idx_entity + 1;
    return true;
  }


  void create_material_from_template(
    Material *material,
    MaterialTemplate *material_template,
    BuiltinTextures *builtin_textures,
    MemoryPool *memory_pool
  ) {
    init_material(material, material_template->name);

    material->albedo_static = material_template->albedo_static;
    material->metallic_static = material_template->metallic_static;
    material->roughness_static = material_template->roughness_static;
    material->ao_static = material_template->ao_static;

    if (!str::is_empty(material_template->shader_asset_vert_path)) {
      shaders::init_shader_asset(
        &material->shader_asset,
        memory_pool,
        material_template->name,
        ShaderType::standard,
        material_template->shader_asset_vert_path,
        material_template->shader_asset_frag_path,
        material_template->shader_asset_geom_path
      );
    }
    if (!str::is_empty(material_template->depth_shader_asset_vert_path)) {
       shaders::init_shader_asset(
        &material->depth_shader_asset,
        memory_pool,
        material_template->name,
        ShaderType::depth,
        material_template->depth_shader_asset_vert_path,
        material_template->depth_shader_asset_frag_path,
        material_template->depth_shader_asset_geom_path
      );
    }

    for (uint32 idx = 0; idx < material_template->n_textures; idx++) {
      Texture texture;
      init_texture(
        &texture,
        material_template->texture_types[idx],
        material_template->texture_paths[idx]
      );
      add_texture_to_material(
        material,
        texture,
        material_template->texture_uniform_names[idx]
      );
    }

    for (uint32 idx = 0; idx < material_template->n_builtin_textures; idx++) {
      const char *builtin_texture_name =
        material_template->builtin_texture_names[idx];
      if (strcmp(builtin_texture_name, "g_position_texture") == 0) {
        add_texture_to_material(
          material, *builtin_textures->g_position_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "g_albedo_texture") == 0) {
        add_texture_to_material(
          material, *builtin_textures->g_albedo_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "shadowmaps_3d") == 0) {
        add_texture_to_material(
          material, *builtin_textures->shadowmaps_3d_texture, builtin_texture_name
        );
      } else if (strcmp(builtin_texture_name, "shadowmaps_2d") == 0) {
        add_texture_to_material(
          material, *builtin_textures->shadowmaps_2d_texture, builtin_texture_name
        );
      } else {
        logs::fatal(
          "Attempted to use unsupported built-in texture %s",
          builtin_texture_name
        );
      }
    }
  }


  void create_model_loader_from_entity_template(
    EntityTemplate *entity_template,
    EntityHandle entity_handle,
    Array<ModelLoader> *model_loaders
  ) {
    ModelLoader *model_loader = model_loaders->push();

    models::init_model_loader(
      model_loader,
      entity_template->model_source,
      entity_template->model_path_or_builtin_model_name
    );

    assert(
      sizeof(model_loader->material_names) == sizeof(entity_template->material_names)
    );
    memcpy(
      &model_loader->material_names,
      &entity_template->material_names,
      sizeof(entity_template->material_names)
    );
  }


  void create_entity_loader_from_entity_template(
    EntityTemplate *entity_template,
    EntityHandle entity_handle,
    EntityLoaderSet *entity_loader_set
  ) {
    EntityLoader *entity_loader = entity_loader_set->loaders[entity_handle];
    models::init_entity_loader(
      entity_loader,
      entity_template->entity_debug_name,
      entity_template->model_path_or_builtin_model_name,
      entity_template->render_pass,
      entity_handle
    );
    entity_loader->spatial_component = entity_template->spatial_component;
    entity_loader->light_component = entity_template->light_component;
    entity_loader->behavior_component = entity_template->behavior_component;
    entity_loader->physics_component = entity_template->physics_component;
  }
}

using peony_parser::MaterialTemplate, peony_parser::EntityTemplate,
  peony_parser::PropValueType, peony_parser::PropValue;
