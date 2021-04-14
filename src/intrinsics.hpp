#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#define KB_TO_B(value) ((value) * 1024ULL)
#define MB_TO_B(value) (KB_TO_B(value) * 1024ULL)
#define GB_TO_B(value) (MB_TO_B(value) * 1024ULL)
#define TB_TO_B(value) (GB_TO_B(value) * 1024ULL)
#define B_TO_KB(value) ((value) / 1024ULL)
#define B_TO_MB(value) (B_TO_KB(value) / 1024ULL)
#define B_TO_GB(value) (B_TO_MB(value) / 1024ULL)
#define B_TO_TB(value) (B_TO_GB(value) / 1024ULL)
#define LEN(x) (sizeof((x)) / sizeof((x)[0]))
// NOTE: This is vulnerable to double evaluation. Be aware or fix.
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define DEG_TO_RAD(deg) ((deg) * PI / 180.0f)
#define RAD_TO_DEG(rad) ((rad) * 180.0f / PI)
#define DEG_TO_RAD32(deg) ((deg) * PI32 / 180.0f)
#define RAD32_TO_DEG(rad) ((rad) * 180.0f / PI32)

#define for_range_named(idx, start, end) \
  for ( \
    uint32 idx = (start); \
    idx < (end); \
    idx++ \
  )

#define for_range(start, end) for_range_named(idx, start, end)

#define for_each(el, set) \
  for ( \
    auto el = (set).begin(); \
    el < (set).end(); \
    el++ \
  ) \

#endif
