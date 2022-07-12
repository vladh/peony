// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include "constants.hpp"
#include "types.hpp"

chrono::steady_clock::time_point debug_start_timer();
real64 debug_end_timer(chrono::steady_clock::time_point t0);

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
