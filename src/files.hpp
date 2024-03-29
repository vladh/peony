// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

class files {
public:
    static unsigned char * load_image(
        char const *path, i32 *width, i32 *height, i32 *n_channels, bool should_flip
    );
    static unsigned char * load_image(
        char const *path, i32 *width, i32 *height, i32 *n_channels
    );
    static void free_image(unsigned char *image_data);
    static u32 get_file_size(char const *path);
    static char const * load_file(memory::Pool *memory_pool, char const *path);
    static char const * load_file(char *string, char const *path);
};
