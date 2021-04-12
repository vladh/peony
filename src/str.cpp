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
