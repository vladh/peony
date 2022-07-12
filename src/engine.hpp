// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <chrono>
namespace chrono = std::chrono;
#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "lights.hpp"
#include "behavior.hpp"
#include "physics.hpp"
#include "models.hpp"
#include "renderer.hpp"
#include "mats.hpp"
#include "core.hpp"
#include "cameras.hpp"


static constexpr uint32 DT_HIST_LENGTH = 512;


class engine {
public:
    struct TimingInfo {
        chrono::steady_clock::time_point frame_start;
        chrono::steady_clock::time_point last_frame_start;
        chrono::nanoseconds frame_duration;
        chrono::steady_clock::time_point time_frame_should_end;

        chrono::steady_clock::time_point second_start;
        uint32 n_frames_this_second;
        uint32 n_frames_since_start;
    };

    struct PerfCounters {
        real64 dt_average;
        real64 dt_hist[DT_HIST_LENGTH];
        uint32 dt_hist_idx;
        uint32 last_fps;
    };

    struct State {
        bool32 is_manual_frame_advance_enabled;
        bool32 should_manually_advance_to_next_frame;
        bool32 should_stop;
        bool32 should_pause;
        bool32 should_limit_fps;
        char current_scene_name[MAX_COMMON_NAME_LENGTH];
        // NOTE: `t` and `dt` will not change when gameplay is paused.
        real64 t; // us
        real64 dt; // us
        real64 timescale_diff;
        PerfCounters perf_counters;
        uint32 n_valid_model_loaders;
        uint32 n_valid_entity_loaders;
        bool32 is_world_loaded;
        bool32 was_world_ever_loaded;
        Array<models::ModelLoader> model_loaders;
        models::EntityLoaderSet entity_loader_set;
        EntitySet entity_set;
        drawable::ComponentSet drawable_component_set;
        lights::ComponentSet light_component_set;
        spatial::ComponentSet spatial_component_set;
        behavior::ComponentSet behavior_component_set;
        anim::ComponentSet animation_component_set;
        physics::ComponentSet physics_component_set;
    };

    static engine::State * todo_fixme_get_engine_state();
    static models::EntityLoader * get_entity_loader(EntityHandle entity_handle);
    static models::ModelLoader * push_model_loader();
    static u32 get_last_drawn_shader_program();
    static void set_last_drawn_shader_program(u32 val);
    static void mark_first_non_internal_handle();
    static EntitySet * get_entity_set();
    static Array<Entity> * get_entities();
    static Array<drawable::Component> * get_drawable_components();
    static Array<lights::Component> * get_light_components();
    static Array<spatial::Component> * get_spatial_components();
    static Array<behavior::Component> * get_behavior_components();
    static Array<anim::Component> * get_animation_components();
    static Array<physics::Component> * get_physics_components();
    static Entity * get_entity(EntityHandle entity_handle);
    static drawable::Component * get_drawable_component(EntityHandle entity_handle);
    static lights::Component * get_light_component(EntityHandle entity_handle);
    static spatial::Component * get_spatial_component(EntityHandle entity_handle);
    static behavior::Component * get_behavior_component(EntityHandle entity_handle);
    static anim::Component * get_animation_component(EntityHandle entity_handle);
    static physics::Component * get_physics_component(EntityHandle entity_handle);
    static real64 get_t();
    static real64 get_dt();
    static void run_main_loop(GLFWwindow *window, WindowSize *window_size);
    static void init(engine::State *engine_state, MemoryPool *asset_memory_pool);

private:
    static engine::State *state;
    static void destroy_model_loaders();
    static void destroy_non_internal_entities();
    static void destroy_scene();
    static bool32 load_scene(const char *scene_name);
    static void handle_console_command();
    static void update_light_position(real32 amount);
    static void process_input(GLFWwindow *window);
    static bool32 check_all_entities_loaded();
    static void update(WindowSize *window_size);
    static TimingInfo init_timing_info(uint32 target_fps);
    static void update_timing_info(TimingInfo *timing, uint32 *last_fps);
    static void update_dt_and_perf_counters(TimingInfo *timing);
};
