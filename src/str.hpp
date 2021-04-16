#ifndef STR_HPP
#define STR_HPP

namespace Str {
  void split_on_first_occurrence(
    const char *str,
    char *part1, size_t part1_length,
    char *part2, size_t part2_length,
    const char separator
  );
  void clear(char *str);
  bool32 is_empty(const char *str);
  bool32 eq(const char *str1, const char *str2);
  bool32 starts_with_character(const char *str, const char character);
  bool32 starts_with(const char *str1, const char *str2);
};

#endif
