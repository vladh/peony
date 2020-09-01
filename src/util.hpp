#ifndef UTIL_H
#define UTIL_H

#include "types.hpp"


unsigned char* util_load_image(const char* path, int32 *width, int32 *height, int32 *n_channels);
char* util_load_file(const char* path);

#endif
