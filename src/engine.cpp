// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include <chrono>
namespace chrono = std::chrono;
#include <thread>
#include "../src_external/pstr.h"
#include "util.hpp"
#include "engine.hpp"
#include "logs.hpp"
#include "debug.hpp"
#include "peony_parser.hpp"
#include "peony_parser_utils.hpp"
#include "models.hpp"
#include "constants.hpp"
#include "internals.hpp"
#include "renderer.hpp"
#include "intrinsics.hpp"


engine::State *engine::state = nullptr;


// This should only be used for printing things out for debugging
engine::State *
engine::debug_get_engine_state()
{
    return engine::state;
}


models::EntityLoader *
engine::get_entity_loader(entities::Handle entity_handle)
{
    return engine::state->entity_loaders[entity_handle];
}


models::ModelLoader *
engine::push_model_loader()
{
    return engine::state->model_loaders.push();
}


f64
engine::get_t()
{
    return engine::state->t;
}


f64
engine::get_dt()
{
    return engine::state->dt;
}


u32 engine::get_frame_number()
{
    return engine::state->timing_info.n_frames_since_start;
}


void
engine::run_main_loop(GLFWwindow *window)
{
    while (!engine::state->should_stop) {
        glfwPollEvents();
        process_input(window);

        if (
            !engine::state->is_manual_frame_advance_enabled ||
            engine::state->should_manually_advance_to_next_frame
        ) {
            update_timing_info(&engine::state->perf_counters.last_fps);

            // If we should pause, stop time-based events.
            if (!engine::state->should_pause) {
                update_dt_and_perf_counters();
            }

            update();
            renderer::render(window);

            if (engine::state->is_manual_frame_advance_enabled) {
                engine::state->should_manually_advance_to_next_frame = false;
            }

            input::reset_n_mouse_button_state_changes_this_frame();
            input::reset_n_key_state_changes_this_frame();

            if (engine::state->should_limit_fps) {
                std::this_thread::sleep_until(
                    engine::state->timing_info.time_frame_should_end);
            }

            #if USE_PRINT_FPS
            logs::info("%u fps", engine::state->perf_counters.last_fps);
            #endif
        }


        if (glfwWindowShouldClose(window)) {
            engine::state->should_stop = true;
        }
    }
}


void engine::init(engine::State *engine_state, memory::Pool *asset_memory_pool) {
    engine::state = engine_state;
    engine::state->model_loaders = Array<models::ModelLoader>(
        asset_memory_pool, MAX_N_MODELS, "model_loaders");
    engine::state->entity_loaders = Array<models::EntityLoader>(
        asset_memory_pool, MAX_N_ENTITIES, "entity_loaders", true, 1);
    engine::state->timing_info = init_timing_info(165);

}


void
engine::destroy_model_loaders()
{
    engine::state->model_loaders.clear();
}


void
engine::destroy_scene()
{
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!engine::state->is_world_loaded) {
        logs::info("Cannot load or unload scene while loading is already in progress.");
        return;
    }

    // TODO: Also reclaim texture names from TextureNamePool, otherwise we'll
    // end up overflowing.
    destroy_model_loaders();
    mats::destroy_non_internal_materials();

    entities::destroy_non_internal_entities();
    engine::state->entity_loaders.delete_elements_after_index(
        entities::get_first_non_internal_handle());
}


bool
engine::load_scene(const char *scene_name)
{
    // If the current scene has not finished loading, we can neither
    // unload it nor load a new one.
    if (!engine::state->is_world_loaded) {
        gui::log("Cannot load or unload scene while loading is already in progress.");
        return false;
    }

    // Get some memory for everything we need
    memory::Pool temp_memory_pool = {};
    defer { memory::destroy_memory_pool(&temp_memory_pool); };

    // Load scene file
    char scene_path[MAX_PATH] = {};
    pstr_vcat(scene_path, MAX_PATH, SCENE_DIR, scene_name, SCENE_EXTENSION, NULL);
    gui::log("Loading scene: %s", scene_path);

    peony_parser::PeonyFile *scene_file = MEMORY_PUSH(&temp_memory_pool,
        peony_parser::PeonyFile, "scene_file");
    if (!peony_parser::parse_file(scene_file, scene_path)) {
        gui::log("Could not load scene: %s", scene_path);
        return false;
    }

    // Destroy our current scene after we've confirmed we could load the new scene.
    destroy_scene();
    pstr_copy(engine::state->current_scene_name, MAX_COMMON_NAME_LENGTH, scene_name);

    // Get only the unique used materials
    Array<char[MAX_COMMON_NAME_LENGTH]> used_materials(
        &temp_memory_pool, MAX_N_MATERIALS, "used_materials");
    peony_parser_utils::get_unique_string_values_for_prop_name(
        scene_file, &used_materials, "materials");

    // Create Materials
    peony_parser::PeonyFile *material_file = MEMORY_PUSH(&temp_memory_pool,
        peony_parser::PeonyFile, "material_file");
    each (used_material, used_materials) {
        memset(material_file, 0, sizeof(peony_parser::PeonyFile));
        char material_file_path[MAX_PATH] = {};
        pstr_vcat(material_file_path, MAX_PATH,
            MATERIAL_FILE_DIRECTORY, *used_material, MATERIAL_FILE_EXTENSION, nullptr);
        if (!peony_parser::parse_file(material_file, material_file_path)) {
            gui::log("Could not load material: %s", material_file_path);
            break;
        }
        assert(material_file->n_entries > 0);
        peony_parser_utils::create_material_from_peony_file_entry(
            mats::push_material(),
            &material_file->entries[0],
            &temp_memory_pool
        );
    }

    range (0, scene_file->n_entries) {
        peony_parser::Entry *entry = &scene_file->entries[idx];

        // Create entities::Entity
        entities::Entity *entity = entities::add_entity_to_set(entry->name);

        // Create models::ModelLoader
        char const *model_path = peony_parser_utils::get_string(
            peony_parser_utils::find_prop(entry, "model_path")
        );
        // NOTE: We only want to make a models::ModelLoader from this peony_parser::Entry if we haven't
        // already encountered this model in a previous entry. If two entities
        // have the same `model_path`, we just make one model and use it in both.
        models::ModelLoader *found_model_loader = engine::state->model_loaders.find(
            [model_path](models::ModelLoader *candidate_model_loader) -> bool {
                return pstr_eq(model_path, candidate_model_loader->model_path);
            }
        );
        if (found_model_loader) {
            logs::info("Skipping already-loaded model %s", model_path);
        } else {
            peony_parser_utils::create_model_loader_from_peony_file_entry(
                entry, entity->handle, engine::state->model_loaders.push());
        }

        // Create models::EntityLoader
        peony_parser_utils::create_entity_loader_from_peony_file_entry(
            entry, entity->handle,
            engine::state->entity_loaders[entity->handle]);
    }

    return true;
}


void
engine::handle_console_command()
{
    char *text_input = input::get_text_input();
    gui::log("%s%s", gui::CONSOLE_SYMBOL, text_input);

    char command[input::MAX_TEXT_INPUT_COMMAND_LENGTH] = {};
    char arguments[input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH] = {};

    pstr_split_on_first_occurrence(text_input,
        command, input::MAX_TEXT_INPUT_COMMAND_LENGTH,
        arguments, input::MAX_TEXT_INPUT_ARGUMENTS_LENGTH,
        ' ');

    if (pstr_eq(command, "help")) {
        gui::log(
            "Some useful commands\n"
            "--------------------\n"
            "loadscene <scene_name>: Load a scene\n"
            "renderdebug <internal_texture_name>: Display an internal texture. "
            "Use texture \"none\" to disable.\n"
            "help: show help"
        );
    } else if (pstr_eq(command, "loadscene")) {
        load_scene(arguments);
    } else if (pstr_eq(command, "renderdebug")) {
        renderer::set_renderdebug_displayed_texture_type(
            mats::texture_type_from_string(arguments));
    } else {
        gui::log("Unknown command: %s", command);
    }

    pstr_clear(text_input);
}


void
engine::update_light_position(f32 amount)
{
    each (light_component, *lights::get_components()) {
        if (light_component->type == lights::LightType::directional) {
            lights::adjust_dir_light_angle(amount);
            break;
        }
    }
}


void
engine::process_input(GLFWwindow *window)
{
    // NOTE: We need to enable text input one frame after the actual key
    // is pressed to enable it, so that text starts being processed another
    // frame after that. This is because, on Linux, the backtick character
    // pressed to enable text input also then immediately gets processed as
    // text, which is not what we want.
    if (engine::state->should_enable_text_input) {
        input::enable_text_input();
        engine::state->should_enable_text_input = false;
    }

    if (input::is_key_now_down(GLFW_KEY_GRAVE_ACCENT)) {
        if (gui::is_console_enabled()) {
            gui::set_console_enabled(false);
            input::disable_text_input();
        } else {
            gui::set_console_enabled(true);
            engine::state->should_enable_text_input = true;
        }
    }

    if (input::is_key_now_down(GLFW_KEY_ENTER)) {
        handle_console_command();
    }

    if (input::is_key_now_down(GLFW_KEY_BACKSPACE)) {
        input::do_text_input_backspace();
    }

    if (input::is_key_now_down(GLFW_KEY_ESCAPE)) {
        input::clear_text_input();
    }

    if (input::is_text_input_enabled()) {
        // If we're typing text in, don't run any of the following stuff.
        return;
    }

    // Continuous
    if (input::is_key_down(GLFW_KEY_W)) {
        cameras::move_front_back(cameras::get_main(), 1, engine::get_dt());
    }

    if (input::is_key_down(GLFW_KEY_S)) {
        cameras::move_front_back(cameras::get_main(), -1, engine::get_dt());
    }

    if (input::is_key_down(GLFW_KEY_A)) {
        cameras::move_left_right(cameras::get_main(), -1, engine::get_dt());
    }

    if (input::is_key_down(GLFW_KEY_D)) {
        cameras::move_left_right(cameras::get_main(), 1, engine::get_dt());
    }

    if (input::is_key_down(GLFW_KEY_Z)) {
        update_light_position(0.10f * (f32)(engine::get_dt()));
    }

    if (input::is_key_down(GLFW_KEY_X)) {
        update_light_position(-0.10f * (f32)(engine::get_dt()));
    }

    if (input::is_key_down(GLFW_KEY_SPACE)) {
        cameras::move_up_down(cameras::get_main(), 1, engine::get_dt());
    }

    if (input::is_key_down(GLFW_KEY_LEFT_CONTROL)) {
        cameras::move_up_down(cameras::get_main(), -1, engine::get_dt());
    }

    // Transient
    if (input::is_key_now_down(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (input::is_key_now_down(GLFW_KEY_C)) {
        input::set_is_cursor_enabled(!input::is_cursor_enabled());
        renderer::update_drawing_options(window);
    }

    if (input::is_key_now_down(GLFW_KEY_R)) {
        load_scene(engine::state->current_scene_name);
    }

    if (input::is_key_now_down(GLFW_KEY_TAB)) {
        engine::state->should_pause = !engine::state->should_pause;
    }

    if (input::is_key_now_down(GLFW_KEY_MINUS)) {
        engine::state->timescale_diff -= 0.1f;
    }

    if (input::is_key_now_down(GLFW_KEY_EQUAL)) {
        engine::state->timescale_diff += 0.1f;
    }

    if (input::is_key_now_down(GLFW_KEY_BACKSPACE)) {
        renderer::set_should_hide_ui(!renderer::should_hide_ui());
    }

    if (input::is_key_down(GLFW_KEY_N)) {
        engine::state->should_manually_advance_to_next_frame = true;
    }

    if (input::is_key_now_down(GLFW_KEY_0)) {
        *((volatile unsigned int*)0) = 0xDEAD;
    }
}


bool
engine::check_all_entities_loaded()
{
    bool are_all_done_loading = true;

    each (material, *mats::get_materials()) {
        bool is_done_loading = mats::prepare_material_and_check_if_done(material);
        if (!is_done_loading) {
            are_all_done_loading = false;
        }
    }

    u32 new_n_valid_model_loaders = 0;
    each (model_loader, engine::state->model_loaders) {
        if (!models::is_model_loader_valid(model_loader)) {
            continue;
        }
        new_n_valid_model_loaders++;
        bool is_done_loading = models::prepare_model_loader_and_check_if_done(model_loader);
        if (!is_done_loading) {
            are_all_done_loading = false;
        }
    }
    engine::state->n_valid_model_loaders = new_n_valid_model_loaders;

    u32 new_n_valid_entity_loaders = 0;
    each (entity_loader, engine::state->entity_loaders) {
        if (!models::is_entity_loader_valid(entity_loader)) {
            continue;
        }
        new_n_valid_entity_loaders++;

        models::ModelLoader *model_loader = engine::state->model_loaders.find(
            [entity_loader](models::ModelLoader *candidate_model_loader) -> bool {
                return pstr_eq(entity_loader->model_path, candidate_model_loader->model_path);
            }
        );
        if (!model_loader) {
            logs::fatal("Encountered an models::EntityLoader %d for which we cannot find the models::ModelLoader.",
                entity_loader->entity_handle);
        }

        bool is_done_loading = models::prepare_entity_loader_and_check_if_done(
            entity_loader,
            model_loader);

        // NOTE: If a certain models::EntityLoader is complete, it's done everything it
        // needed to and we don't need it anymore.
        if (is_done_loading) {
            // TODO: We need to do this in a better way. We should somehow let the
            // Array know when we delete one of these. Even though it's sparse,
            // it should have length 0 if we know there's nothing in it. That way
            // we don't have to iterate over it over and over.
            memset(entity_loader, 0, sizeof(models::EntityLoader));
        }

        if (!is_done_loading) {
            are_all_done_loading = false;
        }
    }
    engine::state->n_valid_entity_loaders = new_n_valid_entity_loaders;

    return are_all_done_loading;
}


void
engine::update()
{
    if (engine::state->is_world_loaded && !engine::state->was_world_ever_loaded) {
        load_scene(DEFAULT_SCENE);
        engine::state->was_world_ever_loaded = true;
    }

    cameras::update_matrices(cameras::get_main());

    engine::state->is_world_loaded = check_all_entities_loaded();

    lights::update(cameras::get_main()->position);
    behavior::update();
    anim::update();
    physics::update();
}


engine::TimingInfo
engine::init_timing_info(u32 target_fps)
{
    TimingInfo timing_info = {};
    timing_info.second_start = chrono::steady_clock::now();
    timing_info.frame_start = chrono::steady_clock::now();
    timing_info.last_frame_start = chrono::steady_clock::now();
    timing_info.frame_duration = chrono::nanoseconds(
        (u32)((f64)1.0f / (f64)target_fps)
    );
    return timing_info;
}


void
engine::update_timing_info(u32 *last_fps)
{
    auto *timing = &engine::state->timing_info;
    timing->n_frames_since_start++;
    timing->last_frame_start = timing->frame_start;
    timing->frame_start = chrono::steady_clock::now();
    timing->time_frame_should_end = timing->frame_start + timing->frame_duration;

    timing->n_frames_this_second++;
    chrono::duration<f64> time_since_second_start =
        timing->frame_start - timing->second_start;
    if (time_since_second_start >= chrono::seconds(1)) {
        timing->second_start = timing->frame_start;
        *last_fps = timing->n_frames_this_second;
        timing->n_frames_this_second = 0;
    }
}


void
engine::update_dt_and_perf_counters()
{
    engine::state->dt = util::get_us_from_duration(
        engine::state->timing_info.frame_start - engine::state->timing_info.last_frame_start);
    if (engine::state->timescale_diff != 0.0f) {
        engine::state->dt *= max(1.0f + engine::state->timescale_diff, (f64)0.01f);
    }

    engine::state->perf_counters.dt_hist[engine::state->perf_counters.dt_hist_idx] =
        engine::state->dt;
    engine::state->perf_counters.dt_hist_idx++;
    if (engine::state->perf_counters.dt_hist_idx >= DT_HIST_LENGTH) {
        engine::state->perf_counters.dt_hist_idx = 0;
    }
    f64 dt_hist_sum = 0.0f;
    for (u32 idx = 0; idx < DT_HIST_LENGTH; idx++) {
        dt_hist_sum += engine::state->perf_counters.dt_hist[idx];
    }
    engine::state->perf_counters.dt_average = dt_hist_sum / DT_HIST_LENGTH;

    engine::state->t += engine::state->dt;
}
