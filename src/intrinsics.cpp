#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#define LEN(x) (sizeof((x)) / sizeof((x)[0]))

// Loops

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

// Defer macro/thing.

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

#endif
