namespace PeonyFileParser {
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
        log_error("Reached max token length.");
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
    } while (is_token_whitespace(token) || token[0] == TOKEN_ELEMENT_SEPARATOR);
    return could_get_token;
  }


  void parse_header(char *token, FILE *f) {
    get_non_trivial_token(token, f);
    assert(is_token_name(token));
    log_info("Header: %s", token);
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
      // NOTE: Type names the can start a value: vec2, vec3, vec4
      if (strcmp(token, "vec2") == 0) {
        prop_value_types[n_values] = PropValueType::vec2;
        parse_vec2(token, f, &prop_values[n_values].vec2_value);
        n_values++;
      } else if (strcmp(token, "vec3") == 0) {
        prop_value_types[n_values] = PropValueType::vec3;
        parse_vec3(token, f, &prop_values[n_values].vec3_value);
        n_values++;
      } else if (strcmp(token, "vec4") == 0) {
        prop_value_types[n_values] = PropValueType::vec4;
        parse_vec4(token, f, &prop_values[n_values].vec4_value);
        n_values++;
      } else {
        prop_value_types[n_values] = PropValueType::string;
        strcpy(prop_values[n_values].string_value, token);
        n_values++;
      }
    } else if (token[0] == TOKEN_ARRAY_START) {
      while (true) {
        get_non_trivial_token(token, f);
        if (token[0] == TOKEN_ARRAY_END) {
          break;
        }
        prop_value_types[n_values] = PropValueType::string;
        strcpy(prop_values[n_values].string_value, token);
        n_values++;
      }
    }

    return n_values;
  }


  void parse_scene_file(const char *path) {
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

        log_info("%s =", prop_name);
        for (uint32 idx_value = 0; idx_value < n_values; idx_value++) {
          if (prop_value_types[idx_value] == PropValueType::string) {
            log_info("  - %s", prop_values[idx_value].string_value);
          } else if (prop_value_types[idx_value] == PropValueType::vec2) {
            log_vec2(&prop_values[idx_value].vec2_value);
          } else if (prop_value_types[idx_value] == PropValueType::vec3) {
            log_vec3(&prop_values[idx_value].vec3_value);
          } else if (prop_value_types[idx_value] == PropValueType::vec4) {
            log_vec4(&prop_values[idx_value].vec4_value);
          } else {
            log_warning("OOPS");
          }
        }
      } else {
        log_info("Unhandled token: %s", token);
      }
    }

    fclose(f);
  }


  void test() {
    parse_scene_file("data/scenes/demo.peony_scene");
    /* parse_scene_file("data/materials/rocks.peony_materials"); */
  }
}
