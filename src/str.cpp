void Str::split_on_first_occurrence(
  const char *str,
  char *part1, size_t part1_length,
  char *part2, size_t part2_length,
  const char separator
) {
  size_t str_length = strlen(str);
  for (uint32 idx = 0; idx < str_length; idx++) {
    // We don't want a space at the very start or end of the string;
    // this wouldn't leave us with much to split. Just ignore those.
    if (str[idx] == separator && idx > 0 && idx < str_length - 1) {
      //           i
      // [str1l    ] [str2l]
      // 0 1 2 3 4 5 6 7 8 9
      // h e l l o   t h e 0
      assert(idx < part1_length);
      assert(str_length - idx - 1 <= part2_length);
      strncpy(part1, str, idx);
      part1[idx] = '\0';
      strncpy(part2, str + idx + 1, str_length - idx - 1);
      part2[str_length - idx - 1] = '\0';
      return;
    }
    // If we haven't found the separator, just put everything into the command.
    if (idx == str_length - 1) {
      assert(part1_length >= str_length);
      strncpy(part1, str, str_length);
      return;
    }
  }
}


void Str::clear(char *str) {
  str[0] = '\0';
}


bool32 Str::is_empty(const char *str) {
  return str[0] == '\0';
}


bool32 Str::eq(const char *str1, const char *str2) {
  return strcmp(str1, str2) == 0;
}


bool32 Str::starts_with_character(const char *str, const char character) {
  return str[0] == character;
}


bool32 Str::starts_with(const char *str, const char *prefix) {
  size_t str_len = strlen(str);
  size_t prefix_len = strlen(prefix);
  if (str_len < prefix_len) {
    return false;
  }
  return memcmp(str, prefix, prefix_len) == 0;
}
