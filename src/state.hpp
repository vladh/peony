#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "lights.hpp"
#include "behavior.hpp"
#include "anim.hpp"
#include "models.hpp"
#include "gui.hpp"
#include "cameras.hpp"
#include "debugdraw.hpp"
#include "physics.hpp"
#include "tasks.hpp"
#include "anim.hpp"
#include "memory.hpp"
#include "array.hpp"
#include "stackarray.hpp"
#include "queue.hpp"

namespace state {
  constexpr uint32 DT_HIST_LENGTH = 512;

  struct WindowSize {
    int32 width; // in pixels (size of framebuffer)
    int32 height; // in pixels (size of framebuffer)
    uint32 screencoord_width; // in screen coordinates
    uint32 screencoord_height; // in screen coordinates
  };

  // TODO: Move to Engine
  struct PerfCounters {
    real64 dt_average;
    real64 dt_hist[DT_HIST_LENGTH];
    uint32 dt_hist_idx;
    uint32 last_fps;
  };

  // TODO: Move to Renderer
  struct ShaderCommon {
    m4 view;
    m4 projection;
    m4 ui_projection;
    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];

    v3 camera_position;
    float camera_pitch;

    float camera_horizontal_fov;
    float camera_vertical_fov;
    float camera_near_clip_dist;
    float camera_far_clip_dist;

    int n_point_lights;
    int n_directional_lights;
    int current_shadow_light_idx;
    int current_shadow_light_type;

    float shadow_far_clip_dist;
    bool is_blur_horizontal;
    TextureType renderdebug_displayed_texture_type;
    int unused_pad;

    float exposure;
    float t;
    int window_width;
    int window_height;

    v4 point_light_position[MAX_N_LIGHTS];
    v4 point_light_color[MAX_N_LIGHTS];
    v4 point_light_attenuation[MAX_N_LIGHTS];

    v4 directional_light_position[MAX_N_LIGHTS];
    v4 directional_light_direction[MAX_N_LIGHTS];
    v4 directional_light_color[MAX_N_LIGHTS];
    v4 directional_light_attenuation[MAX_N_LIGHTS];
  };

  struct State {
    GLFWwindow *window;
    WindowSize window_size;

    // Engine stuff
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
    uint32 first_non_internal_material_idx;
    Array<ModelLoader> model_loaders;
    EntityLoaderSet entity_loader_set;
    EntitySet entity_set;
    DrawableComponentSet drawable_component_set;
    LightComponentSet light_component_set;
    SpatialComponentSet spatial_component_set;
    BehaviorComponentSet behavior_component_set;
    AnimationComponentSet animation_component_set;
    PhysicsComponentSet physics_component_set;

    // Renderer stuff
    bool32 is_cursor_enabled;
    bool32 should_hide_ui;
    bool32 should_use_wireframe;
    TextureType renderdebug_displayed_texture_type;
    ShaderAsset standard_depth_shader_asset;
    uint32 ubo_shader_common;
    ShaderCommon shader_common;
    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];
    BuiltinTextures builtin_textures;

    CamerasState cameras_state;
    InputState input_state;
    GuiState gui_state;
    DebugDrawState debug_draw_state;
    LightsState lights_state;
    AnimState anim_state;
    MaterialsState materials_state;
    TasksState tasks_state;
  };

  struct MemoryAndState {
    MemoryPool *asset_memory_pool;
    State *state;
  };
}

using state::PerfCounters, state::ShaderCommon, state::State,
  state::MemoryAndState, state::WindowSize;
