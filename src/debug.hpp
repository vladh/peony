#ifndef DEBUG_H
#define DEBUG_H

#if USE_TIMERS

#define START_TIMER(name) \
  std::chrono::steady_clock::time_point debug_timerstart_##name = std::chrono::steady_clock::now();

#define END_TIMER_MIN(name, min_duration_ms) \
  std::chrono::duration<real64, std::milli> debug_timerduration_##name = ( \
    std::chrono::steady_clock::now() - debug_timerstart_##name \
  ); \
  if (debug_timerduration_##name.count() >= min_duration_ms) { \
    log_info("Timer %s took %f", #name, debug_timerduration_##name.count()); \
  }

#define END_TIMER(name) END_TIMER_MIN(name, 0)

#else

#define START_TIMER(name)
#define END_TIMER_MIN(name, min_duration_ms)
#define END_TIMER(name)

#endif

#endif
