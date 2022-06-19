/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include <stdarg.h>
#include <assimp/cimport.h>
#include "types.hpp"

class logs {
public:
    static void fatal(const char *format, ...);
    static void error(const char *format, ...);
    static void warning(const char *format, ...);
    static void info(const char *format, ...);
    static void print_newline();
    static void print_aimatrix4x4(aiMatrix4x4 *t);
    static void print_m4(m4 *t);
    static void print_v2(v2 *t);
    static void print_v3(v3 *t);
    static void print_v4(v4 *t);
};
