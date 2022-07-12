// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

class peony_parser {
public:
    static constexpr u32 MAX_N_FILE_ENTRIES = 128;
    static constexpr u32 MAX_N_ENTRY_PROPS = 32;
    static constexpr u32 MAX_N_ARRAY_VALUES = 8;
    static constexpr u32 MAX_TOKEN_LENGTH = 128;

    static constexpr const char TOKEN_SPACE = ' ';
    static constexpr const char TOKEN_NEWLINE = '\n';
    static constexpr const char TOKEN_EQUALS = '=';
    static constexpr const char TOKEN_HEADER_START = '>';
    static constexpr const char TOKEN_ARRAY_START = '[';
    static constexpr const char TOKEN_ARRAY_END = ']';
    static constexpr const char TOKEN_OBJECT_START = '{';
    static constexpr const char TOKEN_OBJECT_END = '}';
    static constexpr const char TOKEN_TUPLE_START = '(';
    static constexpr const char TOKEN_TUPLE_END = ')';
    static constexpr const char TOKEN_ELEMENT_SEPARATOR = ',';
    static constexpr const char TOKEN_COMMENT_START = ';';

    enum class PropValueType {
        unknown, string, boolean, number, vec2, vec3, vec4
    };

    struct PropValue {
        PropValueType type;
        union {
            char string_value[MAX_TOKEN_LENGTH];
            bool boolean_value;
            f32 number_value;
            v2 vec2_value;
            v3 vec3_value;
            v4 vec4_value;
        };
    };

    struct Prop {
        char name[MAX_TOKEN_LENGTH];
        PropValue values[MAX_N_ARRAY_VALUES];
        u32 n_values;
    };

    struct Entry {
        char name[MAX_TOKEN_LENGTH];
        Prop props[MAX_N_ENTRY_PROPS];
        u32 n_props;
    };

    struct PeonyFile {
        Entry entries[MAX_N_FILE_ENTRIES];
        u32 n_entries;
    };

    static bool parse_file(PeonyFile *pf, char const *path);

private:
    static void print_value(PropValue *value);
    static bool is_char_whitespace(const char target);
    static bool is_token_whitespace(const char *token);
    static bool is_char_allowed_in_name(const char target);
    static bool is_token_name(const char *token);
    static bool is_char_token_boundary(char target);
    static bool get_token(char *token, FILE *f);
    static bool get_non_trivial_token(char *token, FILE *f);
    static void parse_vec2(char *token, FILE *f, v2 *parsed_vector);
    static void parse_vec3(char *token, FILE *f, v3 *parsed_vector);
    static void parse_vec4(char *token, FILE *f, v4 *parsed_vector);
    static void get_value_from_token(char *token, FILE *f, PropValue *prop_value);
    static void parse_header(char *token, FILE *f);
    static void parse_property(Prop *prop, char *token, FILE *f);
};
