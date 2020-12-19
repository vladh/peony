#ifndef PEONY_FILE_PARSER_HPP
#define PEONY_FILE_PARSER_HPP

namespace PeonyFileParser {
  constexpr uint32 MAX_TOKEN_LENGTH = 128;
  constexpr uint32 MAX_N_ARRAY_VALUES = 32;

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

  enum class PropValueType {string, vec2, vec3, vec4};

  union PropValue {
    char string_value[MAX_TOKEN_LENGTH];
    glm::vec2 vec2_value;
    glm::vec3 vec3_value;
    glm::vec4 vec4_value;
  };

  bool32 is_char_whitespace(const char target);
  bool32 is_token_whitespace(const char *token);
  bool32 is_char_allowed_in_name(const char target);
  bool32 is_token_name(const char *token);
  bool32 is_char_token_boundary(char target);
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
  void parse_scene_file(const char *path);
};

#endif
