#ifndef UTIL_H
#define UTIL_H

unsigned char* util_load_image(const char *path, int32 *width, int32 *height, int32 *n_channels);
void util_free_image(unsigned char *image_data);
const char* util_load_two_files(MemoryPool *pool, const char *path1, const char* path2);
const char* util_load_file(MemoryPool *pool, const char *path);
const char* util_join(char* buf, const char* s1, const char* s2, const char* s3);
const char* util_join(char* buf, const char* prefix, uint32 n, const char* suffix);
real64 util_random(real64 min, real64 max);
void util_sleep(real64 s);

#endif
