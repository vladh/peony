#ifndef TYPES_H
#define TYPES_H

#define global_variable static
#define local_persist static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

#define PI32 3.14159265359f
#define PI 3.14159265358979323846

#define KB_TO_B(value) ((value) * 1024ULL)
#define MB_TO_B(value) (KB_TO_B(value) * 1024ULL)
#define GB_TO_B(value) (MB_TO_B(value) * 1024ULL)
#define TB_TO_B(value) (GB_TO_B(value) * 1024ULL)
#define B_TO_KB(value) ((value) / 1024ULL)
#define B_TO_MB(value) (B_TO_KB(value) / 1024ULL)
#define B_TO_GB(value) (B_TO_MB(value) / 1024ULL)
#define B_TO_TB(value) (B_TO_GB(value) / 1024ULL)
#define LEN(x) (sizeof(x) / sizeof((x)[0]))
// NOTE: This is vulnerable to double evaluation. Be aware or fix.
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// TODO: I'm sure there's a better way to do this, but this works.

#define START_TIMER(name) \
  std::chrono::steady_clock::time_point debug_timerstart_##name = std::chrono::steady_clock::now();

#define END_TIMER_MIN(name, min_duration_ms) \
  std::chrono::duration<real64> debug_timerduration_##name = std::chrono::steady_clock::now() - debug_timerstart_##name; \
  if (debug_timerduration_##name >= std::chrono::milliseconds(min_duration_ms)) { \
    log_warning("Timer %s took %f", #name, debug_timerduration_##name); \
  }

#define END_TIMER(name) END_TIMER_MIN(name, 0)

#endif
