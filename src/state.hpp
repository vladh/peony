#ifndef STATE_HPP
#define STATE_HPP

constexpr uint32 DT_HIST_LENGTH = 512;

struct WindowInfo {
  GLFWwindow *window;
  uint32 width;
  uint32 height;
  char title[128];
};

class State {
public:
  bool32 is_manual_frame_advance_enabled;
  bool32 should_manually_advance_to_next_frame;
  bool32 should_stop;
  bool32 should_pause;
  bool32 should_hide_ui;
  WindowInfo window_info;

  Cameras::Camera camera_main;
  Cameras::Camera *camera_active;
  Input::InputState input_state;

  real64 t;
  real64 dt;
  real64 dt_average;
  real64 dt_hist[DT_HIST_LENGTH];
  uint32 dt_hist_idx;
  uint32 last_fps;
  uint32 n_frames_since_start;

  bool32 is_cursor_disabled;
  bool32 should_limit_fps;
  bool32 should_use_wireframe;
  glm::vec4 background_color;

  Array<Shaders::ShaderAsset> shader_assets;
  Array<Models::ModelAsset> model_assets;

  Array<Entities::Entity> entities;
  Array<Entities::DrawableComponent> drawable_components;
  Array<Entities::LightComponent> light_components;
  Array<Entities::SpatialComponent> spatial_components;
  Array<Entities::BehaviorComponent> behavior_components;
  Gui::GuiState gui_state;

  EntitySets::EntitySet entity_set;
  EntitySets::DrawableComponentSet drawable_component_set;
  EntitySets::LightComponentSet light_component_set;
  EntitySets::SpatialComponentSet spatial_component_set;
  EntitySets::BehaviorComponentSet behavior_component_set;

  real32 heading_opacity;
  const char *heading_text;
  real32 heading_fadeout_duration;
  real32 heading_fadeout_delay;

  Materials::PersistentPbo persistent_pbo;
  Materials::TextureNamePool texture_name_pool;
  Queue<Tasks::Task> task_queue;

  uint32 cube_shadowmap_width;
  uint32 cube_shadowmap_height;
  uint32 texture_shadowmap_width;
  uint32 texture_shadowmap_height;
  real32 shadowmap_near_clip_dist;
  real32 shadowmap_far_clip_dist;

  real32 dir_light_angle;

  uint32 cube_shadowmaps_framebuffer;
  uint32 cube_shadowmaps;
  uint32 texture_shadowmaps_framebuffer;
  uint32 texture_shadowmaps;
  glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS];
  glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS];
  Materials::Texture *cube_shadowmaps_texture;
  Materials::Texture *texture_shadowmaps_texture;

  Shaders::ShaderAsset *standard_depth_shader_asset;

  uint32 ubo_shader_common;
  Renderer::ShaderCommon shader_common;

  uint32 g_buffer;
  Materials::Texture *g_position_texture;
  Materials::Texture *g_normal_texture;
  Materials::Texture *g_albedo_texture;
  Materials::Texture *g_pbr_texture;

  uint32 l_buffer;
  Materials::Texture *l_color_texture;
  Materials::Texture *l_bright_color_texture;
  Materials::Texture *l_depth_texture;

  uint32 blur1_buffer;
  uint32 blur2_buffer;
  Materials::Texture *blur1_texture;
  Materials::Texture *blur2_texture;

  State(
    MemoryPool *asset_memory_pool,
    MemoryPool *entity_memory_pool,
    WindowInfo window_info
  );
};

struct MemoryAndState {
  MemoryPool *asset_memory_pool;
  MemoryPool *entity_memory_pool;
  State *state;
};

#endif
