// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "debug.hpp"


chrono::steady_clock::time_point debug_start_timer() {
  return chrono::steady_clock::now();
}


real64 debug_end_timer(chrono::steady_clock::time_point t0) {
  return (
    (chrono::duration<real64, std::milli>)(chrono::steady_clock::now() - t0)
  ).count();
}
