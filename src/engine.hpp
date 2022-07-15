// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include "types.hpp"
#include "entities.hpp"
#include "lights.hpp"
#include "behavior.hpp"
#include "physics.hpp"
#include "models.hpp"
#include "renderer.hpp"
#include "mats.hpp"
#include "core.hpp"
#include "cameras.hpp"


static constexpr u32 DT_HIST_LENGTH = 512;


class engine {
public:
    struct TimingInfo {
        chrono::steady_clock::time_point frame_start;
        chrono::steady_clock::time_point last_frame_start;
        chrono::nanoseconds frame_duration;
        chrono::steady_clock::time_point time_frame_should_end;

        chrono::steady_clock::time_point second_start;
        u32 n_frames_this_second;
        u32 n_frames_since_start;
    };

    struct PerfCounters {
        f64 dt_average;
        f64 dt_hist[DT_HIST_LENGTH];
        u32 dt_hist_idx;
        u32 last_fps;
    };

    struct State {
        bool is_manual_frame_advance_enabled;
        bool should_manually_advance_to_next_frame;
        bool should_stop;
        bool should_pause;
        bool should_limit_fps;
        bool should_enable_text_input;
        char current_scene_name[MAX_COMMON_NAME_LENGTH];
        // NOTE: `t` and `dt` will not change when gameplay is paused.
        f64 t; // us
        f64 dt; // us
        f64 timescale_diff;
        PerfCounters perf_counters;
        u32 n_valid_model_loaders;
        u32 n_valid_entity_loaders;
        bool is_world_loaded;
        bool was_world_ever_loaded;
        Array<models::ModelLoader> model_loaders;
        Array<models::EntityLoader> entity_loaders;
        TimingInfo timing_info;
    };

    static engine::State * debug_get_engine_state();
    static models::EntityLoader * get_entity_loader(entities::Handle entity_handle);
    static models::ModelLoader * push_model_loader();
    static f64 get_t();
    static f64 get_dt();
    static u32 get_frame_number();
    static void run_main_loop(GLFWwindow *window);
    static void init(engine::State *engine_state, memory::Pool *asset_memory_pool);

private:
    static engine::State *state;
    static void destroy_model_loaders();
    static void destroy_scene();
    static bool load_scene(const char *scene_name);
    static void handle_console_command();
    static void update_light_position(f32 amount);
    static void process_input(GLFWwindow *window);
    static bool check_all_entities_loaded();
    static void update();
    static TimingInfo init_timing_info(u32 target_fps);
    static void update_timing_info(u32 *last_fps);
    static void update_dt_and_perf_counters();
};
