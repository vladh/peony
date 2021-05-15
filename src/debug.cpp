chrono::steady_clock::time_point debug_start_timer() {
  return chrono::steady_clock::now();
}

real64 debug_end_timer(chrono::steady_clock::time_point t0) {
  return (
    (chrono::duration<real64, std::milli>)(chrono::steady_clock::now() - t0)
  ).count();
}

#if USE_TIMERS

#define START_TIMER(name) \
  auto debug_timerstart_##name = debug_start_timer();

#define END_TIMER_MIN(name, min_duration_ms) \
  { \
    real64 duration = debug_end_timer(debug_timerstart_##name); \
    if (duration >= min_duration_ms) { \
      logs::info("Timer %s took %0.fms", #name, duration); \
    } \
  }

#define END_TIMER(name) END_TIMER_MIN(name, 0)

#else

#define START_TIMER(name)
#define END_TIMER_MIN(name, min_duration_ms)
#define END_TIMER(name)

#endif