#pragma once

#include "types.hpp"

namespace peony_parser {
  constexpr uint32 MAX_N_FILE_ENTRIES = 128;
  constexpr uint32 MAX_N_ENTRY_PROPS = 32;
  constexpr uint32 MAX_N_ARRAY_VALUES = 8;
  constexpr uint32 MAX_TOKEN_LENGTH = 128;

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

  enum class PeonyFilePropValueType {
    unknown, string, boolean, number, vec2, vec3, vec4
  };

  struct PeonyFilePropValue {
    PeonyFilePropValueType type;
    union {
      char string_value[MAX_TOKEN_LENGTH];
      bool32 boolean_value;
      real32 number_value;
      v2 vec2_value;
      v3 vec3_value;
      v4 vec4_value;
    };
  };

  struct PeonyFileProp {
    char name[MAX_TOKEN_LENGTH];
    PeonyFilePropValue values[MAX_N_ARRAY_VALUES];
    uint32 n_values;
  };

  struct PeonyFileEntry {
    char name[MAX_TOKEN_LENGTH];
    PeonyFileProp props[MAX_N_ENTRY_PROPS];
    uint32 n_props;
  };

  struct PeonyFile {
    PeonyFileEntry entries[MAX_N_FILE_ENTRIES];
    uint32 n_entries;
  };

  bool32 parse_file(PeonyFile *pf, char const *path);
}

using peony_parser::PeonyFileEntry,
  peony_parser::PeonyFileProp,
  peony_parser::PeonyFilePropValue,
  peony_parser::PeonyFile;
