// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "../src_external/glad/glad.h"
#include "types.hpp"

class glutil {
public:
    static char const * stringify_glenum(GLenum thing);
    static GLenum get_texture_format_from_n_components(i32 n_components);
    static void print_texture_internalformat_info(GLenum internal_format);
    static void APIENTRY debug_message_callback(
        GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
        char const *message, const void *userParam
    );
};
