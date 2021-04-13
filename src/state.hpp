#ifndef STATE_HPP
#define STATE_HPP

constexpr uint32 DT_HIST_LENGTH = 512;

struct WindowInfo {
  GLFWwindow *window;
  uint32 width;
  uint32 height;
  char title[128];
};

struct PerfCounters {
  real64 dt_average;
  real64 dt_hist[DT_HIST_LENGTH];
  uint32 dt_hist_idx;
  uint32 last_fps;
};

struct BoneMatrixPool {
  Array<glm::mat4> bone_matrices;
  Array<real64> times;
  uint32 n_bone_matrix_sets;
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

  // NOTE: `t` and `dt` will not change when gameplay is paused.
  real64 t; // us
  real64 dt; // us
  PerfCounters perf_counters;

  bool32 is_cursor_enabled;
  bool32 should_limit_fps;
  bool32 should_use_wireframe;
  glm::vec4 background_color;
  uint32 n_valid_model_loaders;
  uint32 n_valid_entity_loaders;

  bool32 is_world_loaded;

  Array<Material> materials;
  Array<ModelLoader> model_loaders;
  uint32 first_non_internal_material_idx;

  GuiState gui_state;

  EntitySet entity_set;
  EntityLoaderSet entity_loader_set;
  DrawableComponentSet drawable_component_set;
  LightComponentSet light_component_set;
  SpatialComponentSet spatial_component_set;
  BehaviorComponentSet behavior_component_set;
  AnimationComponentSet animation_component_set;

  BoneMatrixPool bone_matrix_pool;

  real32 heading_opacity;
  const char *heading_text;
  real32 heading_fadeout_duration;
  real32 heading_fadeout_delay;

  PersistentPbo persistent_pbo;
  TextureNamePool texture_name_pool;
  Queue<Task> task_queue;

  real32 dir_light_angle;

  ShaderAsset standard_depth_shader_asset;

  uint32 ubo_shader_common;
  ShaderCommon shader_common;

  glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS];
  glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS];
  BuiltinTextures builtin_textures;
};


struct MemoryAndState {
  MemoryPool *asset_memory_pool;
  State *state;
};

State* init_state(
  State *state,
  MemoryPool *asset_memory_pool,
  WindowInfo window_info
);

#endif
