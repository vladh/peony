namespace PeonyFileParser {
  void print_material_template(MaterialTemplate *material_template) {
    log_info("shader_asset_vert_path: %s", material_template->shader_asset_vert_path);
    log_info("shader_asset_frag_path: %s", material_template->shader_asset_frag_path);
    log_info("shader_asset_geom_path: %s", material_template->shader_asset_geom_path);
    log_info(
      "depth_shader_asset_vert_path: %s",
      material_template->depth_shader_asset_vert_path
    );
    log_info(
      "depth_shader_asset_frag_path: %s",
      material_template->depth_shader_asset_frag_path
    );
    log_info(
      "depth_shader_asset_geom_path: %s",
      material_template->depth_shader_asset_geom_path
    );
    log_info("albedo_static:");
    log_vec4(&material_template->albedo_static);
    log_info("metallic_static: %f", material_template->metallic_static);
    log_info("roughness_static: %f", material_template->roughness_static);
    log_info("ao_static: %f", material_template->ao_static);
    log_info("n_textures: %d", material_template->n_textures);
    for (
      uint32 idx_texture = 0;
      idx_texture < material_template->n_textures;
      idx_texture++
    ) {
      log_info(
        "texture %s (%s, %s)",
        material_template->texture_uniform_names[idx_texture],
        Textures::texture_type_to_string(
          material_template->texture_types[idx_texture]
        ),
        material_template->texture_paths[idx_texture]
      );
    }
    log_info("n_builtin_textures: %d", material_template->n_builtin_textures);
    for (
      uint32 idx_texture = 0;
      idx_texture < material_template->n_builtin_textures;
      idx_texture++
    ) {
      log_info(
        "built-in texture %s",
        material_template->builtin_texture_names[idx_texture]
      );
    }
  }


  void print_entity_template(EntityTemplate *entity_template) {
    log_info("name: %s", entity_template->entity_debug_name);
    log_info("model_path: %s", entity_template->model_path);
    log_info("builtin_model_name: %s", entity_template->builtin_model_name);
    log_info("n_materials: %d", entity_template->n_materials);
    log_info("{");
    for (
      uint32 idx_material = 0;
      idx_material < entity_template->n_materials;
      idx_material++
    ) {
      print_material_template(&entity_template->material_templates[idx_material]);
    }
    log_info("}");
    log_info("render_pass: %d", entity_template->render_pass);
    entity_template->spatial_component.print();
  }


  void print_value(PropValue value, PropValueType type) {
    if (type == PropValueType::unknown) {
      log_info("<unknown>");
    } else if (type == PropValueType::string) {
      log_info("%s", value.string_value);
    } else if (type == PropValueType::boolean) {
      log_info("%d", value.boolean_value);
    } else if (type == PropValueType::number) {
      log_info("%f", value.number_value);
    } else if (type == PropValueType::vec2) {
      log_vec2(&value.vec2_value);
    } else if (type == PropValueType::vec3) {
      log_vec3(&value.vec3_value);
    } else if (type == PropValueType::vec4) {
      log_vec4(&value.vec4_value);
    } else {
      log_info("<invalid>");
    }
  }


  void init_material_template(MaterialTemplate *material_template) {
    material_template->albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
    material_template->metallic_static = -1.0f;
    material_template->roughness_static = -1.0f;
    material_template->ao_static = -1.0f;
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
    for (uint32 idx_char = 0; idx_char < strlen(token); idx_char++) {
      if (!is_char_allowed_in_name(token[idx_char])) {
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


  void get_value_from_token(
    char *token,
    FILE *f,
    PropValueType *prop_value_type,
    PropValue *prop_value
  ) {
    // NOTE: Type names the can start a value: vec2, vec3, vec4
    if (strcmp(token, "vec2") == 0) {
      *prop_value_type = PropValueType::vec2;
      parse_vec2(token, f, &prop_value->vec2_value);
    } else if (strcmp(token, "vec3") == 0) {
      *prop_value_type = PropValueType::vec3;
      parse_vec3(token, f, &prop_value->vec3_value);
    } else if (strcmp(token, "vec4") == 0) {
      *prop_value_type = PropValueType::vec4;
      parse_vec4(token, f, &prop_value->vec4_value);
    } else if (strcmp(token, "true") == 0) {
      *prop_value_type = PropValueType::boolean;
      prop_value->boolean_value = true;
    } else if (strcmp(token, "false") == 0) {
      *prop_value_type = PropValueType::boolean;
      prop_value->boolean_value = false;
    } else if (strcmp(token, "0.0") == 0 || strtod(token, nullptr) != 0.0f) {
      // NOTE: `strtod()` returns 0.0 if parsing fails, so we need to check
      // if our value actually was 0.0;
      *prop_value_type = PropValueType::number;
      prop_value->number_value = (real32)strtod(token, nullptr);
    } else {
      *prop_value_type = PropValueType::string;
      strcpy(prop_value->string_value, token);
    }
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
        log_error("Reached max token length for token %s.", token);
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


  void parse_header(char *token, FILE *f) {
    get_non_trivial_token(token, f);
    assert(is_token_name(token));
  }


  void parse_vec2(char *token, FILE *f, glm::vec2 *parsed_vector) {
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_START);
    get_non_trivial_token(token, f);
    (*parsed_vector).x = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).y = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_END);
  }


  void parse_vec3(char *token, FILE *f, glm::vec3 *parsed_vector) {
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


  void parse_vec4(char *token, FILE *f, glm::vec4 *parsed_vector) {
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


  uint32 parse_property(
    char *token,
    FILE *f,
    char prop_name[MAX_TOKEN_LENGTH],
    PropValueType prop_value_types[MAX_N_ARRAY_VALUES],
    PropValue prop_values[MAX_N_ARRAY_VALUES]
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


  void parse_material_file(
    const char *path, MaterialTemplate *material_template
  ) {
    FILE *f = fopen(path, "r");

    if (!f) {
      log_fatal("Could not open file %s.", path);
      return;
    }

    char token[MAX_TOKEN_LENGTH];
    uint32 n_values;
    char prop_name[MAX_TOKEN_LENGTH];
    PropValueType prop_value_types[MAX_N_ARRAY_VALUES];
    PropValue prop_values[MAX_N_ARRAY_VALUES];

    while (get_non_trivial_token(token, f)) {
      if (token[0] == TOKEN_HEADER_START) {
        parse_header(token, f);
      } else if (is_token_name(token)) {
        n_values = parse_property(
          token, f, prop_name, prop_value_types, prop_values
        );

        if (strcmp(prop_name, "shader_asset.vert_path") == 0) {
          strcpy(
            material_template->shader_asset_vert_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "shader_asset.frag_path") == 0) {
          strcpy(
            material_template->shader_asset_frag_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "shader_asset.geom_path") == 0) {
          strcpy(
            material_template->shader_asset_geom_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "depth_shader_asset.vert_path") == 0) {
          strcpy(
            material_template->depth_shader_asset_vert_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "depth_shader_asset.frag_path") == 0) {
          strcpy(
            material_template->depth_shader_asset_frag_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "depth_shader_asset.geom_path") == 0) {
          strcpy(
            material_template->depth_shader_asset_geom_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "albedo_static") == 0) {
          material_template->albedo_static = prop_values[0].vec4_value;
        } else if (strcmp(prop_name, "metallic_static") == 0) {
          material_template->metallic_static =
            prop_values[0].number_value;
        } else if (strcmp(prop_name, "roughness_static") == 0) {
          material_template->roughness_static =
            prop_values[0].number_value;
        } else if (strcmp(prop_name, "ao_static") == 0) {
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
            Textures::texture_type_from_string(prop_values[0].string_value);
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
          log_info("Unhandled prop_name %s with values:", prop_name);
          for (uint32 idx_value = 0; idx_value < n_values; idx_value++) {
            print_value(prop_values[idx_value], prop_value_types[idx_value]);
          }
        }
      } else {
        log_info("Unhandled token: %s", token);
      }
    }

    fclose(f);
  }


  uint32 parse_scene_file(
    const char *path, EntityTemplate *entity_templates
  ) {
    int32 idx_entity = -1;

    FILE *f = fopen(path, "r");

    if (!f) {
      log_fatal("Could not open file %s.", path);
      return 0;
    }

    EntityTemplate *entity_template = nullptr;
    char token[MAX_TOKEN_LENGTH];
    uint32 n_values;
    char prop_name[MAX_TOKEN_LENGTH];
    PropValueType prop_value_types[MAX_N_ARRAY_VALUES];
    PropValue prop_values[MAX_N_ARRAY_VALUES];

    while (get_non_trivial_token(token, f)) {
      if (token[0] == TOKEN_HEADER_START) {
        idx_entity++;
        entity_template = &entity_templates[idx_entity];
        parse_header(token, f);
        strcpy(
          entity_template->entity_debug_name,
          token
        );
      } else if (is_token_name(token)) {
        n_values = parse_property(
          token, f, prop_name, prop_value_types, prop_values
        );

        if (strcmp(prop_name, "model_path") == 0) {
          strcpy(
            entity_template->model_path,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "builtin_model_name") == 0) {
          strcpy(
            entity_template->builtin_model_name,
            prop_values[0].string_value
          );
        } else if (strcmp(prop_name, "materials") == 0) {
          entity_template->n_materials = n_values;
          for (uint32 idx_value = 0; idx_value < n_values; idx_value++) {
            char material_file_path[MAX_TOKEN_LENGTH];
            strcpy(material_file_path, MATERIAL_FILE_DIRECTORY);
            strcat(material_file_path, prop_values[idx_value].string_value);
            strcat(material_file_path, MATERIAL_FILE_EXTENSION);
            init_material_template(&entity_template->material_templates[idx_value]);
            parse_material_file(
              material_file_path,
              &entity_template->material_templates[idx_value]
            );
          }
        } else if (strcmp(prop_name, "render_passes") == 0) {
          Renderer::RenderPassFlag render_pass = Renderer::RenderPass::none;
          for (uint32 idx_value = 0; idx_value < n_values; idx_value++) {
            render_pass = render_pass |
              Renderer::render_pass_from_string(prop_values[idx_value].string_value);
          }
          entity_template->render_pass = render_pass;
        } else if (strcmp(prop_name, "spatial_component.position") == 0) {
          entity_template->spatial_component.position = prop_values[0].vec3_value;
        } else if (strcmp(prop_name, "spatial_component.rotation") == 0) {
          entity_template->spatial_component.rotation =
            glm::angleAxis(
              glm::radians(prop_values[0].vec4_value[0]),
              glm::vec3(
                prop_values[0].vec4_value[1],
                prop_values[0].vec4_value[2],
                prop_values[0].vec4_value[3]
              )
            );
        } else if (strcmp(prop_name, "spatial_component.scale") == 0) {
          entity_template->spatial_component.scale = prop_values[0].vec3_value;
        } else if (strcmp(prop_name, "light_component.type") == 0) {
          entity_template->light_component.type =
            light_type_from_string(prop_values[0].string_value);
        } else if (strcmp(prop_name, "light_component.direction") == 0) {
          entity_template->light_component.direction = prop_values[0].vec3_value;
        } else if (strcmp(prop_name, "light_component.color") == 0) {
          entity_template->light_component.color = prop_values[0].vec4_value;
        } else if (strcmp(prop_name, "light_component.attenuation") == 0) {
          entity_template->light_component.attenuation = prop_values[0].vec4_value;
        } else if (strcmp(prop_name, "behavior_component.behavior") == 0) {
          entity_template->behavior_component.behavior =
            behavior_from_string(prop_values[0].string_value);
        } else {
          log_info("Unhandled prop_name %s with values:", prop_name);
          for (uint32 idx_value = 0; idx_value < n_values; idx_value++) {
            print_value(prop_values[idx_value], prop_value_types[idx_value]);
          }
        }
      } else {
        log_info("Unhandled token: %s", token);
      }
    }

    fclose(f);

    return idx_entity + 1;
  }
}
