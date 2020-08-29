#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;
typedef float real32;
typedef double real64;

#define global_variable static
#define local_persist static
#define internal static

typedef struct State {
  uint32 window_width;
  uint32 window_height;
  char window_title[32];
  real32 test_vertices[128];
  real32 test_indices[128];
  uint32 shader_program;
  uint32 vao;
  uint32 test_texture;
} State;
