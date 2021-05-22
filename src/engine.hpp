#pragma once

namespace engine {
  struct TimingInfo {
    chrono::steady_clock::time_point frame_start;
    chrono::steady_clock::time_point last_frame_start;
    chrono::nanoseconds frame_duration;
    chrono::steady_clock::time_point time_frame_should_end;

    chrono::steady_clock::time_point second_start;
    uint32 n_frames_this_second;
    uint32 n_frames_since_start;
  };

  void run_main_loop(State *state);
  void init(State *state);
}
