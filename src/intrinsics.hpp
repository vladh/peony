#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#define LEN(x) (sizeof((x)) / sizeof((x)[0]))

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
