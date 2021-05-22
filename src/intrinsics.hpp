#pragma once

// Flags
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#pragma warning(disable: 4505) // unreferenced local function has been removed


// Functions
#define LEN(x) (sizeof((x)) / sizeof((x)[0]))


// Keywords
#define pny_global static
#define pny_local_persist static
#define pny_internal static


// Loops
#define pny_for_range_named(idx, start, end) \
  for ( \
    uint32 idx = (start); \
    idx < (end); \
    idx++ \
  )

#define pny_for_range(start, end) pny_for_range_named(idx, start, end)

#define pny_for_each(el, set) \
  for ( \
    auto el = (set).begin(); \
    el < (set).end(); \
    el++ \
  ) \


// Defer macro/thing
#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

template<typename T>
struct ExitScope {
  T lambda;
  ExitScope(T lambda) : lambda(lambda) {}
  ~ExitScope() { lambda(); }
  ExitScope(const ExitScope&);
private:
  ExitScope& operator =(const ExitScope&);
};

class ExitScopeHelp {
public:
  template<typename T>
    ExitScope<T> operator+(T t){ return t;}
};

#define defer [[maybe_unused]] const auto& CONCAT(defer__, __LINE__) = \
  ExitScopeHelp() + [&]()
