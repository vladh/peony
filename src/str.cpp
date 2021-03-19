bool32 Str::is_empty(const char *str) {
  return str[0] == '\0';
}


bool32 Str::are_equal(const char *str1, const char *str2) {
  return strcmp(str1, str2) == 0;
}
