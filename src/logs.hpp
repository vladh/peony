#pragma once

#include <stdarg.h>
#include <assimp/cimport.h>
#include "types.hpp"

namespace logs {
  void fatal(const char *format, ...);
  void error(const char *format, ...);
  void warning(const char *format, ...);
  void info(const char *format, ...);
  void print_newline();
  void print_aimatrix4x4(aiMatrix4x4 *t);
  void print_m4(m4 *t);
  void print_v2(v2 *t);
  void print_v3(v3 *t);
  void print_v4(v4 *t);
}
