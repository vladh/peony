/*!
  pstr string functions

  Copyright 2021 Vlad-Stefan Harbuz <vlad@vladh.net>

  Permission is hereby granted, free of charge, to any person obtaining a copy of this
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify,
  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be included in all copies
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


bool pstr_is_valid(char const *str, size_t const size) {
  for (size_t idx = 0; idx < size; idx++) {
    if (str[idx] == 0) {
      return true;
    }
  }
  return false;
}


int64_t pstr_len(char const *str) {
  return strlen(str);
}


bool pstr_is_empty(char const *str) {
  return str[0] == '\0';
}


bool pstr_eq(char const *str1, char const *str2) {
  return strcmp(str1, str2) == 0;
}


bool pstr_starts_with_char(char const *str, char const character) {
  return str[0] == character;
}


bool pstr_starts_with(char const *str, char const *prefix) {
  size_t str_len = pstr_len(str);
  size_t prefix_len = pstr_len(prefix);
  if (str_len == 0 || prefix_len == 0) {
    return false;
  }
  if (str_len < prefix_len) {
    return false;
  }
  return memcmp(str, prefix, prefix_len) == 0;
}


bool pstr_ends_with_char(char const *str, char const character) {
  size_t str_len = pstr_len(str);
  return str[str_len - 1] == character;
}


bool pstr_ends_with(char const *str, char const *prefix) {
  size_t str_len = pstr_len(str);
  size_t prefix_len = pstr_len(prefix);
  if (str_len == 0 || prefix_len == 0) {
    return false;
  }
  if (str_len < prefix_len) {
    return false;
  }
  return memcmp(str + str_len - prefix_len, prefix, prefix_len) == 0;
}


bool pstr_copy(char *dest, size_t const dest_size, char const *src) {
  size_t const src_len = pstr_len(src);

  // If there's no room, return false
  if (dest_size < src_len + 1) {
    return false;
  }

  memcpy(dest, src, src_len);
  dest[src_len] = '\0';

  return true;
}


bool pstr_copy_n(char *dest, size_t const dest_size, char const *src, size_t const n) {
  size_t const src_len = pstr_len(src);

  // If there's no room, return false
  if (src_len < n || dest_size < n + 1) {
    return false;
  }

  memcpy(dest, src, n);
  dest[n] = '\0';

  return true;
}


bool pstr_cat(char *dest, size_t const dest_size, char const *src) {
  size_t const src_len = pstr_len(src);
  size_t const dest_len = pstr_len(dest);
  size_t const free_size = dest_size - dest_len;

  // If there's no room, return false
  if (free_size < src_len + 1 || src_len == 0) {
    return false;
  }

  memcpy(dest + dest_len, src, src_len);
  dest[dest_len + src_len] = '\0';

  return true;
}


bool pstr_vcat(char *dest, size_t const dest_size, ...) {
  size_t const dest_len = pstr_len(dest);
  size_t free_size = dest_size - dest_len;
  char *cursor = dest + dest_len;

  va_list args;
  va_start(args, dest_size);

  char const *src;

  while (true) {
    src = va_arg(args, char const*);
    if (!src) {
      break;
    }
    size_t const src_len = pstr_len(src);

    // If there's no room, return false
    if (free_size < src_len + 1 || src_len == 0) {
      // Restore our string to what it was before
      dest[dest_len] = 0;
      return false;
    }

    memcpy(cursor, src, src_len);
    cursor += src_len;
    free_size -= src_len;
  }

  *cursor = '\0';

  return true;
}


bool pstr_split_on_first_occurrence(
  char const *src,
  char *part1, size_t const part1_size,
  char *part2, size_t const part2_size,
  char const separator
) {
  size_t const src_len = pstr_len(src);

  // Find separator
  char const *separator_start = strchr(src, separator);
  if (!separator_start) {
    return false;
  }
  size_t idx_separator = separator_start - src;

  // Find how much space we need before and after the separator
  size_t const src_len_before_sep = idx_separator;
  size_t const src_len_after_sep = src_len - idx_separator - 1;

  // Return if we don't have enough space
  if (part1_size < src_len_before_sep + 1 || part2_size < src_len_after_sep + 1) {
    return false;
  }

  memcpy(part1, src, src_len_before_sep);
  memcpy(part2, separator_start + 1, src_len_after_sep);

  return true;
}


void pstr_clear(char *str) {
  str[0] = '\0';
}


bool pstr_slice_from(char *str, size_t const start) {
  size_t const str_len = pstr_len(str);
  if (start >= str_len) {
    return false;
  }
  uint32_t idx = 0;
  char *cursor = &str[start];
  do {
    str[idx++] = *cursor;
    cursor++;
  } while (*cursor != 0);
  str[idx++] = 0;
  return true;
}


bool pstr_slice_to(char *str, size_t const end) {
  size_t const str_len = pstr_len(str);
  if (end >= str_len) {
    return false;
  }
  str[end] = 0;
  return true;
}


bool pstr_slice(char *str, size_t const start, size_t const end) {
  if (start >= end) {
    return false;
  }
  return pstr_slice_to(str, end) && pstr_slice_from(str, start);
}


void pstr_ltrim(char *str) {
  size_t n_spaces = 0;
  while (isspace(str[n_spaces])) {
    n_spaces++;
  }
  pstr_slice_from(str, n_spaces);
}


void pstr_rtrim(char *str) {
  size_t str_len = pstr_len(str);
  size_t n_spaces = 0;
  while (isspace(str[str_len - n_spaces - 1])) {
    n_spaces++;
  }
  pstr_slice_to(str, str_len - n_spaces);
}


void pstr_trim(char *str) {
  pstr_ltrim(str);
  pstr_rtrim(str);
}


void pstr_ltrim_char(char *str, char const target) {
  size_t n_matches = 0;
  while (str[n_matches] == target) {
    n_matches++;
  }
  pstr_slice_from(str, n_matches);
}


void pstr_rtrim_char(char *str, char const target) {
  size_t str_len = pstr_len(str);
  size_t n_matches = 0;
  while (str[str_len - n_matches - 1] == target) {
    n_matches++;
  }
  pstr_slice_to(str, str_len - n_matches);
}


void pstr_trim_char(char *str, char const target) {
  pstr_ltrim_char(str, target);
  pstr_rtrim_char(str, target);
}


bool pstr_from_int64(
  char *str, size_t const str_size, int64_t number, size_t *new_str_len
) {
  char *cursor = str;
  *new_str_len = 0;
  uint64_t number_abs = (number < 0) ? -number : number;

  // Make a backwards string, since that's easier to produce
  do {
    *cursor++ = '0' + (number_abs % 10);
    (*new_str_len)++;

    // Check that we have space for the string so far, the NULL terminator and a
    // potential '-' character
    if (*new_str_len > str_size - 2) {
      str[0] = 0;
      return false;
    }

    number_abs /= 10;
  } while(number_abs > 0);

  if (number < 0) {
    *cursor++ = '-';
  }

  *cursor = '\0';

  // Reverse our string so it's the right way around again
  char swapped_char;
  cursor--;
  while(str < cursor) {
    swapped_char = *str;
    *str = *cursor;
    *cursor = swapped_char;
    str++;
    cursor--;
  }

  return true;
}
