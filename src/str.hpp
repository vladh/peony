#ifndef STR_HPP
#define STR_HPP

namespace Str {
  bool32 is_empty(const char *str);
  bool32 eq(const char *str1, const char *str2);
  bool32 starts_with_character(const char *str, const char character);
  bool32 starts_with(const char *str1, const char *str2);
};

#endif
