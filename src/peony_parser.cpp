/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "../src_external/pstr.h"
#include "peony_parser.hpp"
#include "logs.hpp"
#include "intrinsics.hpp"


namespace peony_parser {
  pny_internal void print_value(PeonyFilePropValue *value) {
    if (value->type == PeonyFilePropValueType::unknown) {
      logs::info("<unknown>");
    } else if (value->type == PeonyFilePropValueType::string) {
      logs::info("%s", value->string_value);
    } else if (value->type == PeonyFilePropValueType::boolean) {
      logs::info("%d", value->boolean_value);
    } else if (value->type == PeonyFilePropValueType::number) {
      logs::info("%f", value->number_value);
    } else if (value->type == PeonyFilePropValueType::vec2) {
      logs::print_v2(&value->vec2_value);
    } else if (value->type == PeonyFilePropValueType::vec3) {
      logs::print_v3(&value->vec3_value);
    } else if (value->type == PeonyFilePropValueType::vec4) {
      logs::print_v4(&value->vec4_value);
    } else {
      logs::info("<invalid>");
    }
  }


  pny_internal bool32 is_char_whitespace(const char target) {
    return target == TOKEN_NEWLINE ||
      target == TOKEN_SPACE;
  }


  pny_internal bool32 is_token_whitespace(const char *token) {
    return is_char_whitespace(token[0]);
  }


  pny_internal bool32 is_char_allowed_in_name(const char target) {
    return isalpha(target) ||
      isdigit(target) ||
      target == '_' ||
      target == '-' ||
      target == '/' ||
      target == ':' ||
      target == '.';
  }


  pny_internal bool32 is_token_name(const char *token) {
    range (0, pstr_len(token)) {
      if (!is_char_allowed_in_name(token[idx])) {
        return false;
      }
    }
    return true;
  }


  pny_internal bool32 is_char_token_boundary(char target) {
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


  pny_internal bool32 get_token(char *token, FILE *f) {
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


  pny_internal bool32 get_non_trivial_token(char *token, FILE *f) {
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


  pny_internal void parse_vec2(char *token, FILE *f, v2 *parsed_vector) {
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_START);
    get_non_trivial_token(token, f);
    (*parsed_vector).x = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    (*parsed_vector).y = (real32)strtod(token, nullptr);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_TUPLE_END);
  }


  pny_internal void parse_vec3(char *token, FILE *f, v3 *parsed_vector) {
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


  pny_internal void parse_vec4(char *token, FILE *f, v4 *parsed_vector) {
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


  pny_internal void get_value_from_token(
    char *token, FILE *f, PeonyFilePropValue *prop_value
  ) {
    // NOTE: Type names the can start a value: vec2, vec3, vec4
    if (pstr_eq(token, "vec2")) {
      prop_value->type = PeonyFilePropValueType::vec2;
      parse_vec2(token, f, &prop_value->vec2_value);
    } else if (pstr_eq(token, "vec3")) {
      prop_value->type = PeonyFilePropValueType::vec3;
      parse_vec3(token, f, &prop_value->vec3_value);
    } else if (pstr_eq(token, "vec4")) {
      prop_value->type = PeonyFilePropValueType::vec4;
      parse_vec4(token, f, &prop_value->vec4_value);
    } else if (pstr_eq(token, "true")) {
      prop_value->type = PeonyFilePropValueType::boolean;
      prop_value->boolean_value = true;
    } else if (pstr_eq(token, "false")) {
      prop_value->type = PeonyFilePropValueType::boolean;
      prop_value->boolean_value = false;
    } else if (pstr_eq(token, "0.0") || strtod(token, nullptr) != 0.0f) {
      // NOTE: `strtod()` returns 0.0 if parsing fails, so we need to check
      // if our value actually was 0.0;
      prop_value->type = PeonyFilePropValueType::number;
      prop_value->number_value = (real32)strtod(token, nullptr);
    } else {
      prop_value->type = PeonyFilePropValueType::string;
      pstr_copy(prop_value->string_value, MAX_TOKEN_LENGTH, token);
    }
  }


  pny_internal void parse_header(char *token, FILE *f) {
    get_non_trivial_token(token, f);
    assert(is_token_name(token));
  }


  pny_internal void parse_property(
    PeonyFileProp *prop,
    char *token,
    FILE *f
  ) {
    pstr_copy(prop->name, MAX_TOKEN_LENGTH, token);
    get_non_trivial_token(token, f);
    assert(token[0] == TOKEN_EQUALS);
    get_non_trivial_token(token, f);

    assert(
      is_token_name(token) || token[0] == TOKEN_ARRAY_START
    );

    if (is_token_name(token)) {
      get_value_from_token(token, f, &prop->values[prop->n_values]);
      prop->n_values++;
      if (prop->n_values > MAX_N_ARRAY_VALUES) {
        logs::fatal("Too many array values in peony file");
        assert(false);
      }
    } else if (token[0] == TOKEN_ARRAY_START) {
      while (true) {
        get_non_trivial_token(token, f);
        if (token[0] == TOKEN_ARRAY_END) {
          break;
        }
        get_value_from_token(token, f, &prop->values[prop->n_values]);
        prop->n_values++;
      }
    }
  }
}


bool32 peony_parser::parse_file(PeonyFile *pf, char const *path) {
  int32 idx_entry = -1;
  PeonyFileEntry *entry = nullptr;

  FILE *f = fopen(path, "r");
  if (!f) {
    logs::error("Could not open file %s.", path);
    return false;
  }
  defer { fclose(f); };

  char token[MAX_TOKEN_LENGTH];

  while (get_non_trivial_token(token, f)) {
    if (token[0] == TOKEN_HEADER_START) {
      idx_entry++;
      entry = &pf->entries[idx_entry];
      parse_header(token, f);
      pstr_copy(entry->name, MAX_TOKEN_LENGTH, token);
    } else if (is_token_name(token)) {
      if (!entry) {
        logs::fatal("Tried to parse file, but encountered data before header.");
      }
      PeonyFileProp *prop = &entry->props[entry->n_props];
      parse_property(prop, token, f);
      entry->n_props++;
      if (entry->n_props > MAX_N_ENTRY_PROPS) {
        logs::fatal("Too many props in peony file");
        assert(false);
      }
    } else {
      logs::info("Unhandled token: %s", token);
    }
  }

  pf->n_entries = idx_entry + 1;
  return true;
}
