namespace state {
  // -----------------------------------------------------------
  // Constants
  // -----------------------------------------------------------
  constexpr uint32 DT_HIST_LENGTH = 512;


  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  struct WindowInfo {
    GLFWwindow *window;
    uint32 width; // in pixels (size of framebuffer)
    uint32 height; // in pixels (size of framebuffer)
    uint32 screencoord_width; // in screen coordinates
    uint32 screencoord_height; // in screen coordinates
    char title[128];
  };

  struct PerfCounters {
    real64 dt_average;
    real64 dt_hist[DT_HIST_LENGTH];
    uint32 dt_hist_idx;
    uint32 last_fps;
  };

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
    bool32 is_manual_frame_advance_enabled;
    bool32 should_manually_advance_to_next_frame;
    bool32 should_stop;
    bool32 should_pause;
    bool32 should_hide_ui;
    WindowInfo window_info;

    Camera camera_main;
    Camera *camera_active;
    InputState input_state;
    char current_scene_name[MAX_TOKEN_LENGTH];

    // NOTE: `t` and `dt` will not change when gameplay is paused.
    real64 t; // us
    real64 dt; // us
    real64 timescale_diff;
    PerfCounters perf_counters;

    bool32 is_cursor_enabled;
    bool32 should_limit_fps;
    bool32 should_use_wireframe;
    v4 background_color;
    uint32 n_valid_model_loaders;
    uint32 n_valid_entity_loaders;

    bool32 is_world_loaded;
    bool32 was_world_ever_loaded;

    Array<Material> materials;
    Array<ModelLoader> model_loaders;
    uint32 first_non_internal_material_idx;

    GuiState gui_state;
    DebugDrawState debug_draw_state;
    GameConsole game_console;

    EntitySet entity_set;
    EntityLoaderSet entity_loader_set;
    DrawableComponentSet drawable_component_set;
    LightComponentSet light_component_set;
    SpatialComponentSet spatial_component_set;
    BehaviorComponentSet behavior_component_set;
    AnimationComponentSet animation_component_set;
    PhysicsComponentSet physics_component_set;

    BoneMatrixPool bone_matrix_pool;

    PersistentPbo persistent_pbo;
    TextureNamePool texture_name_pool;
    Queue<Task> task_queue;

    real32 dir_light_angle;
    TextureType renderdebug_displayed_texture_type;

    ShaderAsset standard_depth_shader_asset;

    uint32 ubo_shader_common;
    ShaderCommon shader_common;

    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];
    BuiltinTextures builtin_textures;
  };

  struct MemoryAndState {
    MemoryPool *asset_memory_pool;
    State *state;
  };


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  State* init_state(
    State *state,
    MemoryPool *asset_memory_pool,
    WindowInfo window_info
  ) {
    state->window_info = window_info;

    state->background_color = v4(0.81f, 0.93f, 1.00f, 1.0f);

    state->materials = Array<Material>(
      asset_memory_pool, MAX_N_MATERIALS, "materials"
    );
    state->model_loaders = Array<ModelLoader>(
      asset_memory_pool, MAX_N_MODELS, "model_loaders"
    );

    state->task_queue = Queue<Task>(asset_memory_pool, 128, "task_queue");

    state->builtin_textures = {
      .shadowmap_3d_width = min(state->window_info.width, (uint32)2000),
      .shadowmap_3d_height = min(state->window_info.width, (uint32)2000),
      .shadowmap_2d_width = 2560 * 2,
      .shadowmap_2d_height = 1440 * 2,
      .shadowmap_near_clip_dist = 0.05f,
      .shadowmap_far_clip_dist = 200.0f,
    };

    state->dir_light_angle = radians(55.0f);

    state->entity_loader_set = {
      .loaders = Array<EntityLoader>(
        asset_memory_pool, MAX_N_ENTITIES, "entity_loaders", true, 1
      )
    };
    state->entity_set = {
      .entities = Array<Entity>(
        asset_memory_pool, MAX_N_ENTITIES, "entities", true, 1
      )
    };
    state->drawable_component_set = {
      .components = Array<DrawableComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "drawable_components", true, 1
      )
    };
    state->light_component_set = {
      .components = Array<LightComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "light_components", true, 1
      )
    };
    state->spatial_component_set = {
      .components = Array<SpatialComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "spatial_components", true, 1
      )
    };
    state->behavior_component_set = {
      .components = Array<BehaviorComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "behavior_components", true, 1
      )
    };
    state->animation_component_set = {
      .components = Array<AnimationComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "animation_components", true, 1
      )
    };
    state->physics_component_set = {
      .components = Array<PhysicsComponent>(
        asset_memory_pool, MAX_N_ENTITIES, "physics_components", true, 1
      )
    };
    state->bone_matrix_pool.bone_matrices = Array<m4>(
      asset_memory_pool,
      MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
      "bone_matrices",
      true
    );
    state->bone_matrix_pool.times = Array<real64>(
      asset_memory_pool,
      MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
      "bone_matrix_times",
      true
    );

    gui::g_console = &state->game_console;
    debugdraw::g_dds = &state->debug_draw_state;

    return state;
  }
}

using state::WindowInfo, state::PerfCounters, state::ShaderCommon, state::State,
  state::MemoryAndState;
