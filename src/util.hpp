#ifndef UTIL_H
#define UTIL_H

unsigned char* util_load_image(const char *path, int32 *width, int32 *height, int32 *n_channels);
void util_free_image(unsigned char *image_data);
char* util_load_file(const char *path);
real64 util_random(real64 min, real64 max);

#endif
